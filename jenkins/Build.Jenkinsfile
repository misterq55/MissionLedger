// MissionLedger build pipeline
//
// Jenkins job: Pipeline, "Pipeline script from SCM", Git https://github.com/misterq55/MissionLedger.git,
//              branch */main, Script Path jenkins/Build.Jenkinsfile, started by hand ("Build with Parameters")
// Plugins:     Pipeline, Git, Timestamper, JUnit
// Details:     docs/CI_PLAN.md §5
//
// Jenkins only decides what runs where and in which order. How to build is defined by CMake
// (CMakeLists.txt, CMakePresets.json); every command below can be run the same way on a PC.

import groovy.transform.Field

// Build nodes (node name or label) → platform, which is also the CMake preset prefix (windows-release, ...)
@Field final Map PLATFORMS = [WindowsMini: 'windows', MacMini: 'macos']

// Platforms that build. The others only run the node check (preflight) until their build exists.
// macOS: MissionLedger.app + CLI, smoke tests, unsigned .dmg (roadmap 7, docs/CI_PLAN.md §6)
@Field final List IMPLEMENTED = ['windows', 'macos']

// Platforms with a <platform>-installer build preset (Windows: Inno Setup .exe, macOS: .dmg)
@Field final List PACKAGED = ['windows', 'macos']

// Deploy (docs/CI_PLAN.md §5): a release is uploaded to <DRIVE_REMOTE>:<DRIVE_FOLDER>/v<VERSION>/ and latest/
// with rclone. The rclone.conf is the Jenkins secret file RCLONE_CREDENTIAL (never in the repository).
// The upload runs once, on DEPLOY_NODE, after both platforms succeeded (both installers are stashed there).
@Field final String DRIVE_REMOTE = 'gdrive'
@Field final String DRIVE_FOLDER = 'MissionLedger'
@Field final String RCLONE_CREDENTIAL = 'missionledger-rclone-conf'
@Field final String DEPLOY_NODE = 'WindowsMini'     // also written in the Deploy stage's agent label below

String platformOf(String node) { return PLATFORMS[node] }
boolean isImplemented(String node) { return IMPLEMENTED.contains(platformOf(node)) }
boolean isPackaged(String node) { return PACKAGED.contains(platformOf(node)) }

// Release = the remote tag v<VERSION> points to this commit (jenkins/scripts/release_check.cmake)
boolean isRelease() { return env.ML_RELEASE == 'yes' }
// Upload in this build? A tagged commit deploys unless DEPLOY is off; DEPLOY_DRY_RUN tries it on any commit
boolean deployWanted() {
    return params.DEPLOY && (isRelease() || params.DEPLOY_DRY_RUN) &&
           params.PLATFORM == 'both' && params.CONFIGURATION == 'Release'
}
// A real release always starts from a clean build/ (CLEAN_BUILD does not have to be ticked)
boolean cleanWanted() { return params.CLEAN_BUILD || (deployWanted() && !params.DEPLOY_DRY_RUN) }

// bat on Windows, sh on macOS
void runCmd(String command) {
    if (isUnix()) { sh command } else { bat command }
}

// preflight.cmake runs on CMake itself, so check CMake first: a missing CMake then says what to do
// instead of "command not found". (No ">" or parentheses in the bat echo lines: cmd would parse them.)
void requireCmake() {
    if (isUnix()) {
        sh '''#!/bin/sh
command -v cmake >/dev/null 2>&1 && exit 0
echo "ERROR: cmake is not in the PATH of the Jenkins agent on this node."
echo "Install it on the node (once, by hand):  sh jenkins/scripts/node_setup_macos.sh"
echo "then add its folder to the node's PATH:"
echo "  Manage Jenkins > Nodes > this node > Configure > Environment variables: PATH+CMAKE=/Applications/CMake.app/Contents/bin"
echo "Agent PATH: $PATH"
exit 1
'''
    } else {
        bat '''@echo off
where cmake >nul 2>nul && exit /b 0
echo ERROR: cmake is not in the PATH of the Jenkins agent on this node.
echo Install it on the node once, in an administrator PowerShell:
echo   powershell -NoProfile -ExecutionPolicy Bypass -File jenkins\\scripts\\node_setup_windows.ps1
echo then set PATH+CMAKE=C:\\Program Files\\CMake\\bin in the node settings: Manage Jenkins - Nodes - this node - Configure - Environment variables,
echo or reboot the node: the agent service keeps the PATH it started with.
echo Agent PATH: %PATH%
exit /b 1
'''
    }
}

// Failure notification: stub. Replace the body with mail / a messenger webhook when needed
void notifyResult(String result) {
    echo "[notify] Build ${env.BUILD_NUMBER} (${env.APP_VERSION ?: '?'}): ${result} - ${env.BUILD_URL ?: ''} (notification not set up yet)"
}

pipeline {
    agent none

    parameters {
        choice(name: 'PLATFORM', choices: ['both', 'windows', 'macos'],
               description: 'Platform to build. macOS: app + CLI, smoke tests, unsigned .dmg (roadmap 7)')
        choice(name: 'CONFIGURATION', choices: ['Release', 'Debug'],
               description: 'Release also builds the installer (Windows .exe, macOS .dmg). Debug: build and smoke test only')
        booleanParam(name: 'CLEAN_BUILD', defaultValue: false,
               description: 'Delete build/ first (rebuilds wxWidgets and PDF-Writer, a few minutes longer)')
        booleanParam(name: 'DEPLOY', defaultValue: true,
               description: 'Upload to Google Drive when this commit is a release (tag v<VERSION>, made by release.bat / release.sh). Needs PLATFORM=both and Release; forces a clean build. Untagged commits never upload. Untick to build a tagged commit without uploading')
        booleanParam(name: 'DEPLOY_DRY_RUN', defaultValue: false,
               description: 'Test the Deploy stage on any commit without writing: rclone, the secret file, the remote folder and the release notes are checked, the uploads run with --dry-run. Needs PLATFORM=both and Release')
    }

    options {
        timestamps()
        timeout(time: 120, unit: 'MINUTES')
        disableConcurrentBuilds()
        buildDiscarder(logRotator(numToKeepStr: '20', artifactNumToKeepStr: '5'))
    }

    stages {
        stage('Platforms') {
            matrix {
                // failFast is off: if one platform fails the build is FAILURE, but the other still finishes
                axes {
                    axis {
                        name 'NODE'
                        values 'WindowsMini', 'MacMini'
                    }
                }
                when {
                    beforeAgent true    // a platform that is not selected never waits for its node
                    expression { params.PLATFORM == 'both' || params.PLATFORM == platformOf(env.NODE) }
                }
                agent { label "${NODE}" }
                options {
                    // includes waiting for the node: an offline node fails here (run with PLATFORM=windows then)
                    timeout(time: 60, unit: 'MINUTES')
                }
                stages {
                    stage('Preflight') {
                        steps {
                            requireCmake()
                            script {
                                env.APP_VERSION = readFile('VERSION').trim()
                                // Both cells compute the same answer (same commit); the Deploy stage checks again
                                runCmd 'cmake -DOUT=build/release_check.txt -P jenkins/scripts/release_check.cmake'
                                env.ML_RELEASE = readFile('build/release_check.txt').trim()
                                def tag = isRelease() ? ' release' : ''
                                currentBuild.displayName = "#${env.BUILD_NUMBER} v${env.APP_VERSION} ${params.CONFIGURATION}${tag}"
                                if (isRelease() && params.DEPLOY && !deployWanted()) {
                                    unstable("v${env.APP_VERSION} is a release, but it is not uploaded: that needs PLATFORM=both and CONFIGURATION=Release")
                                }
                                def rclone = (deployWanted() && env.NODE == DEPLOY_NODE) ? '-DREQUIRE_RCLONE=ON' : ''
                                runCmd "cmake -DPLATFORM=${platformOf(env.NODE)} \"-DWORKSPACE=${env.WORKSPACE}\" ${rclone} -P jenkins/scripts/preflight.cmake"
                            }
                        }
                    }
                    stage('Not implemented') {
                        when { expression { !isImplemented(env.NODE) } }
                        steps {
                            echo "${platformOf(env.NODE)}: node check passed; the build itself is not implemented yet (docs/CI_PLAN.md roadmap 7-8)"
                        }
                    }
                    stage('Clean') {
                        when { expression { isImplemented(env.NODE) && cleanWanted() } }
                        steps {
                            dir("build/${platformOf(env.NODE)}") { deleteDir() }
                        }
                    }
                    stage('Configure') {
                        when { expression { isImplemented(env.NODE) } }
                        steps {
                            runCmd "cmake --preset ${platformOf(env.NODE)} -DML_BUILD_NUMBER=${env.BUILD_NUMBER}"
                        }
                    }
                    stage('Build') {
                        when { expression { isImplemented(env.NODE) } }
                        steps {
                            runCmd "cmake --build --preset ${platformOf(env.NODE)}-${params.CONFIGURATION.toLowerCase()} --parallel"
                        }
                    }
                    stage('Test') {
                        when { expression { isImplemented(env.NODE) } }
                        steps {
                            // ctest writes the JUnit file relative to the build folder
                            runCmd "ctest --preset ${platformOf(env.NODE)}-${params.CONFIGURATION.toLowerCase()} --output-junit ctest-junit.xml"
                        }
                        post {
                            always { junit testResults: "build/${platformOf(env.NODE)}/ctest-junit.xml", allowEmptyResults: true }
                        }
                    }
                    stage('Package') {
                        when { expression { isPackaged(env.NODE) && params.CONFIGURATION == 'Release' } }
                        steps {
                            dir('installer/output') { deleteDir() }   // installers of older versions must not be archived again
                            runCmd "cmake --build --preset ${platformOf(env.NODE)}-installer"
                            script {
                                if (deployWanted()) {
                                    stash name: "installer-${platformOf(env.NODE)}", includes: 'installer/output/*'
                                }
                            }
                        }
                    }
                    stage('Archive') {
                        when { expression { isImplemented(env.NODE) } }
                        steps {
                            script {
                                def p = platformOf(env.NODE)
                                def c = params.CONFIGURATION
                                def files = ["build/${p}/tests/out/${c}/*.pdf"]       // smoke test PDFs
                                if (c == 'Release' && isPackaged(env.NODE)) {
                                    files += ['installer/output/*']                     // installer (.exe / .dmg)
                                    if (p == 'windows') {
                                        files += ["build/${p}/bin/Release/*.pdb"]    // PDBs for crash analysis
                                    }
                                }
                                archiveArtifacts artifacts: files.join(', '), allowEmptyArchive: true, fingerprint: true
                            }
                        }
                    }
                }
            }
        }

        // Runs only when both platforms succeeded (a failed matrix stops the pipeline before this stage)
        stage('Deploy') {
            when { expression { deployWanted() } }
            agent { label 'WindowsMini' }   // = DEPLOY_NODE (a field cannot be used in the agent label)
            steps {
                script {
                    if (!params.DEPLOY_DRY_RUN) {
                        // The tag must still point to this commit when the upload starts
                        runCmd 'cmake -DOUT=build/release_check.txt -P jenkins/scripts/release_check.cmake'
                        if (readFile('build/release_check.txt').trim() != 'yes') {
                            error("v${env.APP_VERSION}: the tag no longer points to this commit; nothing uploaded")
                        }
                    }
                    dir('installer/output') { deleteDir() }
                    unstash 'installer-windows'
                    unstash 'installer-macos'
                    def dry = params.DEPLOY_DRY_RUN ? '-DDRY_RUN=ON' : ''
                    withCredentials([file(credentialsId: RCLONE_CREDENTIAL, variable: 'RCLONE_CONFIG')]) {
                        runCmd "cmake -DREMOTE=${DRIVE_REMOTE}:${DRIVE_FOLDER} -DFILES=installer/output -DWORK=build/deploy ${dry} -P jenkins/scripts/deploy.cmake"
                    }
                    if (!params.DEPLOY_DRY_RUN) {
                        currentBuild.description = "Deployed v${env.APP_VERSION}: ${DRIVE_REMOTE}:${DRIVE_FOLDER}/v${env.APP_VERSION}/ and latest/"
                    }
                }
            }
        }
    }

    post {
        failure { notifyResult('FAILURE') }
        fixed   { notifyResult('FIXED') }
    }
}
