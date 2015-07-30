function begin_sign_files {
    param($files, $bindir, $outdir, $approvers, $projectName, $projectUrl, $jobDescription, $jobKeywords, $certificates, [switch] $delaysigned)

    if ($files.Count -eq 0) {
        return
    }
    
    $files = @($files | %{@{path="$bindir\$_"; name=$projectName}} | ?{Test-Path $_.path})
    $approvers = @($approvers | Where-Object {$_ -ne $env:USERNAME})

    [Reflection.Assembly]::Load("CODESIGN.Submitter, Version=3.0.0.6, Culture=neutral, PublicKeyToken=3d8252bd1272440d, processorArchitecture=MSIL") | Out-Null
    [Reflection.Assembly]::Load("CODESIGN.PolicyManager, Version=1.0.0.0, Culture=neutral, PublicKeyToken=3d8252bd1272440d, processorArchitecture=MSIL") | Out-Null

    while ($True) {
        try {
            $job = [CODESIGN.Submitter.Job]::Initialize("codesign.gtm.microsoft.com", 9556, $True)
            $job.Description = $jobDescription
            $job.Keywords = $jobKeywords

            if ($certificates -match "authenticode") {
                $job.SelectCertificate("10006")  # Authenticode
            }
            if ($certificates -match "strongname") {
                $job.SelectCertificate("67")     # StrongName key
            }
            if ($certificates -match "opc") {
                $job.SelectCertificate("160")     # Microsoft OPC Publisher (VSIX)
            }

            foreach ($approver in $approvers) {
                $job.AddApprover($approver)
            }

            foreach ($file in $files) {
                $job.AddFile($file.path, $file.name, $projectUrl, [CODESIGN.JavaPermissionsTypeEnum]::None)
            }

            $job.Send()
            break
        } catch [Exception] {
            echo $_.Exception.Message
            sleep 60
        }
    }
    $jobs += @{job=$job; description=$jobDescription; filecount=$($files.Count); outdir=$outdir}
    end_sign_files $jobs
}

function end_sign_files {
    param($jobs)
    
    if ($jobs.Count -eq 0) {
        return
    }
    
    foreach ($jobinfo in $jobs) {
        $job = $jobinfo.job
        if($job -eq $null) {
            throw "jobinfo in unexpected format $jobinfo"
        }
        $filecount = $jobinfo.filecount
        $outdir = $jobinfo.outdir
        $activity = "Processing $($jobinfo.description) (Job ID $($job.JobID))"
        $percent = 0
        $jobCompletionPath = $job.JobCompletionPath

        if([string]::IsNullOrWhiteSpace($jobCompletionPath)) {
            throw "job.JobCompletionPath is not valid: $job.JobCompletionPath"
        }

        do {
            $files = dir $jobCompletionPath
            Write-Progress -activity $activity -status "Waiting for completion: $jobCompletionPath" -percentcomplete $percent;
            $percent = ($percent + 1) % 100
            sleep -seconds 5
        } while(-not $files -or $files.Count -ne $filecount);
        
        mkdir $outdir -EA 0 | Out-Null
        Write-Progress -Activity $activity -Completed
        
        Write-Output "Copying from $jobCompletionPath to $outdir"
        $retries = 9
        $delay = 2
        $copied = $null
        while ($retries) {
            try {
                $copied = (Copy-Item -path $jobCompletionPath\* -dest $outdir -Force -PassThru)
                break
            } catch {
                if ($retries -eq 0) {
                    break
                }
                Write-Warning "Failed to copy - retrying in $delay seconds ($retries tries remaining)"
                Sleep -seconds $delay
                --$retries
                $delay += $delay
            }
        }
        if (-not $copied) {
            Throw "Failed to copy $jobCompletionPath to $outdir"
        } else {
            Write-Output "Copied $($copied.Count) files"
        }
    }
}
