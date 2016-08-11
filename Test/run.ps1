# Test harness for running node.js tests in a Universal Windows Platform app.

# PARAMETERS:

# appxFolderPath:
#     Location of appx that will run tests. It should also contain a .cer
#     file which will be saved to TrustedPeople store and allow the appx
#     to be installed. See -
#     https://msdn.microsoft.com/en-us/library/windows/apps/bg126232.aspx
#testSrcPath:
#     Test folder of a Node.js clone.
#appLauncherPath:
#     Path to appx launcher


# HOW IT WORKS:

# 1. Install appx cert
# 2. Install appx
# 3. Copy tests to appx storage folder
# 4. Reads .testinfo in documents folder to get list of tests to run
# 5. For each test:
#      a. Create package.json file which contains arguments for node
#      b. Run node
# 6. Uninstall appx

# This script uses the data in nodeuwp.testinfo (that is read from the
# documents folder) to run Node.js tests. The .testinfo file is an xml
# file with the following format:

# <NodeTestInfo>
#   <UseFolders></UseFolders>
#   <Files></Files>
#   <Folders></Folders>
#   <Unsupported></Unsupported>
# </NodeTestInfo>

# UseFolders:
#     Value can be 'true' or 'false' 
#     If true, then the tests run will be the list of scripts in the
#     <Files> element. Scripts should be separated by a white-space character.
#     If false, then the tests in the folders specified in the <Folders>
#     element. Folders should be separated by a white-space character.
# Files:
#     List of scripts to run. The format should be 'test\<test sub folder>\<script>'
# Unsupported:
#     List of strings used to check if a script is should be skipped (i.e. testing
#     an unsupported feature).

# Example .testinfo file:
# <NodeTestInfo>
#   <UseFolders>false</UseFolders>
#     <Files>
#   test\parallel\test-assert.js
#   test\parallel\test-buffer.js
#   test\parallel\test-child-process-buffering.js
#     </Files>
#   <Folders>parallel sequential</Folders>
#   <Unsupported>child-process cluster pipe stdin</Unsupported>
# </NodeTestInfo>

# In the example above, since UseFolders is false, the test-assert and test-buffer
# will be run. If UserFolders is true, then all tests in the 'test\parallel' folder
# will be run. Any script that matches a string in the Unsupported element will be 
# skipped.


# OUTPUT:

# Output for the tests will be saved in the folder where this script is run from.
# If <Files> element is used then the name of the log file is results.log.
# If <Folders> element is used then the log file(s) naming convention is
# results_<folder name>.log.
# (Note: This PowerShell script as well as the node JavaScript tests will log to
# the same file in the appx's local storage folder (nodeuwp.log). At the end
# of the test run, the file is copied to the results*.log file)

# The format of the output is:
# <Timestamp> TestLog: Test count = [<Test count>]
# <Timestamp> TestLog: <Test index>  - Start Test: <Script Name>
# <Timestamp> TestLog: End Test
# ...
# <Timestamp> TestLog: **TEST SUMMARY**
# <Timestamp> TestLog: Tests run = [<Number of tests run>/<Test count>]. Tests passed = [<Number of pass tests>/<Number of tests run>]

# Example output:
# 2016-06-18 15:17:18Z TestLog: Test count = [3]
# 2016-06-18 15:17:18Z TestLog: 1  - Start Test: test\parallel\test-assert.js
# All OK
# Exit Code: 0
# 2016-06-18 15:17:21Z TestLog: End Test
# 2016-06-18 15:17:22Z TestLog: 2  - Start Test: test\parallel\test-buffer.js
# Error: Cannot find module 'C:\Users\munyirik\AppData\Local\Packages\nodeuwpui_gqmz2j608xdxe\LocalState\test\parallel\test-buffer.js'
#    at Module._resolveFilename (module.js:341:5)
#    at Module._load (module.js:290:3)
#    at Module.runMain (module.js:453:3)
#    at startup (node.js:148:11)
#    at Anonymous function (node.js:519:3)
# 2016-06-18 15:17:25Z TestLog: End Test
# 2016-06-18 15:17:25Z TestLog: **TEST SUMMARY**
# 2016-06-18 15:17:25Z TestLog: Tests run = [2/3]. Tests passed = [1/2]


#Requires -RunAsAdministrator

param (
  [string]$app,
  [string]$test,
  [string]$appl
)

function Print-Usage {
  Write-Host "USAGE:"
  Write-Host "run.ps1 -app <Appx path> -test <Test path> -appl <App launcher path>"
}

if([string]::IsNullOrEmpty($app)) {
  Write-Host "Error: Appx folder path required"
  Print-Usage
  Exit
}

if([string]::IsNullOrEmpty($test)) {
  Write-Host "Error: Test path required"
  Print-Usage
  Exit
}

if([string]::IsNullOrEmpty($appl)) {
  Write-Host "Error: App launcher path required"
  Print-Usage
  Exit
}

$appxFolderPath = $app.TrimEnd('\')
$testSrcPath = $test
$appLauncherPath = $appl

# Install appx certificate to TrustedPeople
$cerFileName = Get-Childitem -path $appxFolderPath -filter *.cer
$certPath = $appxFolderPath + '\' + $cerFileName
Import-Certificate -FilePath $certPath -CertStoreLocation cert:\LocalMachine\TrustedPeople


# Install appx
$packageName = Get-AppxPackage -Name nodeuwpui* | Select Name, PackageFullName
If ([string]::IsNullOrWhitespace($packageName)) {
  # Uninstall first if it's installed already
  $appxFileName = Get-Childitem -path $appxFolderPath -filter *.appx
  $appxPath = $appxFolderPath + $appxFileName
  Add-AppxPackage $appxPath
}

$package = Get-AppxPackage -Name nodeuwpui*

# Copy test files to app local storage
$appStoragePath = $env:LOCALAPPDATA + "\Packages\" + $package.PackageFamilyName + "\LocalState\"
Copy-Item -Path $testSrcPath -Destination $appStoragePath -Recurse -Force


$docsFolder = [environment]::getfolderpath("mydocuments")
$startupinfoFileName = "\package.json"

$testInfoFileName = Get-Childitem -path $docsFolder -filter nodeuwp.testinfo
$testInfoPath = $docsFolder + "\" + $testInfoFileName

[xml]$testInfo = Get-Content $testInfoPath

$useFolders = $TRUE

# Name of log file needs to match log file name used by the app
$resultsFile = $appStoragePath + "\nodeuwp.log"

function Log-Message {
Param ($msg)
  $time = Get-Date -format u
  Write-Output "$time TestLog: $msg" | Out-File -FilePath $resultsFile -Append
  Write-Output "" | Out-File -FilePath $resultsFile -Append
}

function Save-StartupInfo {
Param ($t, $f)
  @{name="node-uwp-test";version="0.0.0";main=$t} | ConvertTo-Json -Compress | Out-File $f
}

function Run-Tests {
Param ($ta, $fol)
  $i = 1
  $testCount = $ta.Count
  $testsRun
  Log-Message -msg "Test count = [$testCount]"
  
  $unsupportedTests = $testInfo.NodeTestInfo.Unsupported.split()| where {$_}

  Foreach($t in $ta) {
    # Check if test is supported
    $next = $FALSE
    Foreach($u in $unsupportedTests) {
      If($t -match $u) {
        $next = $TRUE
        break
      }
    }

    if($next) {
      continue
    }
    
    Log-Message -msg "$i  - Start Test: $t"

    $testPath = $docsFolder + $startupinfoFileName
    
    Save-StartupInfo -t $t -f $testPath
    
    $appName = ($package.PackageFamilyName + "!App")
    
    $args = "/appid", $appName
    Start-Process -FilePath $appLauncherPath -ArgumentList $args -Wait
    
    # Wait for app to exit
    $processName = "nodeuwpui"
    $process = Get-Process $processName -ErrorAction SilentlyContinue
    if ($process) {
      Wait-Process -Name $processName -Timeout 60
    }
    
    Log-Message -msg "End Test"
    $i++
  }
  
  $testsRun = $i - 1
  $resultsStr = Get-Content $resultsFile
  $testsPassed = ([regex]::Matches($resultsStr, "Exit Code: 0" )).count

  Log-Message -msg "**TEST SUMMARY**"
  if($fol) {
    Log-Message -msg "Folder name = $fol"
  }
  Log-Message -msg "Tests run = [$testsRun/$testCount]. Tests passed = [$testsPassed/$testsRun]"
}

If("false" -eq $testInfo.NodeTestInfo.UseFolders) {
  $useFolders = $FALSE
}

function Delete-Log {
  # Remove results log file if it exists
  If (Test-Path $resultsFile) {
    Remove-Item $resultsFile
  }
  New-Item $resultsFile
}


if(-Not $useFolders) {
  Delete-Log
  $tests = $testInfo.NodeTestInfo.Files.split()| where {$_}
  Run-Tests -ta $tests
  
  # Copy log file to script location
  Copy-Item -Path $resultsFile -Destination ($PSScriptRoot + "\results.log") -Force
}

if($useFolders) {
  $folders = $testInfo.NodeTestInfo.Folders.split()| where {$_}
  
  Foreach($f in $folders) {
    Delete-Log
    $fullFolderPath = $appStoragePath + "test\" + $f;
    $tests = Get-Childitem -path $fullfolderpath -filter *.js
    
    # Make test follow format "test\<test category>\<js file>"
    for ($i=0; $i -lt $tests.Count; $i++) {
      $tests[$i] = "test\" + $f + "\" + $tests[$i]
    }

    Run-Tests -ta $tests -fol $f
    
    # Copy log file to script location
    Copy-Item -Path $resultsFile -Destination ($PSScriptRoot + "\results_" + $f + ".log") -Force
  }
}

# Clean up
Remove-AppxPackage $package.PackageFullName
#TODO: Uninstall cert as well


