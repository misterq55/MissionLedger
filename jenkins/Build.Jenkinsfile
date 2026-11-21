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

// Deploy (docs/CI_PLAN.md §4–§5), with rclone; the rclone.conf is the Jenkins secret file RCLONE_CREDENTIAL
// (never in the repository). The upload runs once, on DEPLOY_NODE, after the platforms succeeded (their
// installers are stashed there):
//   BUILD=dev      → <DRIVE_REMOTE>:<DRIVE_FOLDER>/dev/<BUILD_NUMBER>-<commit>/, newest DEV_KEEP folders kept
//   BUILD=shipping → <DRIVE_REMOTE>:<DRIVE_FOLDER>/v<VERSION>/ and latest/ (a release)
@Field final String DRIVE_REMOTE = 'gdrive'
@Field final String DRIVE_FOLDER = 'MissionLedger'
@Field final String RCLONE_CREDENTIAL = 'missionledger-rclone-conf'
@Field final String DEPLOY_NODE = 'WindowsMini'     // also written in the Deploy stage's agent label below
@Field final int DEV_KEEP = 5

String platformOf(String node) { return PLATFORMS[node] }
boolean isImplemented(String node) { return IMPLEMENTED.contains(platformOf(node)) }
boolean isPackaged(String node) { return PACKAGED.contains(platformOf(node)) }

// What this build is (parameter BUILD). Empty on the first run after the parameters changed: Jenkins then
// still shows the old parameters, so fall back to the default
String buildKind() { return params.BUILD ?: 'dev' }
// dev, shipping, test = Release build with installers; debug = Debug build, smoke tests only
String configuration() { return buildKind() == 'debug' ? 'Debug' : 'Release' }
boolean isShipping() { return buildKind() == 'shipping' }
// Upload in this build? Only dev and shipping; a tag alone never uploads (it is a precondition of shipping)
boolean uploadWanted() { return buildKind() == 'dev' || isShipping() }
// Release tag = the remote tag v<VERSION> points to this commit (jenkins/scripts/release_check.cmake)
boolean hasReleaseTag() { return env.ML_RELEASE == 'yes' }
// Platforms selected by PLATFORM that make an installer
List packagedSelected() {
    return PACKAGED.findAll { params.PLATFORM == 'both' || params.PLATFORM == it }
}
// A real release always starts from a clean build/ (CLEAN_BUILD does not have to be ticked)
boolean cleanWanted() { return params.CLEAN_BUILD || (isShipping() && !params.DEPLOY_DRY_RUN) }

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
        choice(name: 'BUILD', choices: ['dev', 'shipping', 'test', 'debug'],
               description: 'dev: Release build, installers uploaded to Google Drive MissionLedger/dev/<build>-<commit>/ (newest 5 kept), any commit. ' +
                            'shipping: the release; needs the pushed tag v<VERSION> on this commit (release.bat / release.sh) and PLATFORM=both, clean build, uploads to v<VERSION>/ and latest/. ' +
                            'test: Release build, installers only as Jenkins artifacts. debug: Debug build and smoke tests only')
        booleanParam(name: 'CLEAN_BUILD', defaultValue: false,
               description: 'Delete build/ first (rebuilds wxWidgets and PDF-Writer, a few minutes longer). Always done for BUILD=shipping')
        booleanParam(name: 'DEPLOY_DRY_RUN', defaultValue: false,
               description: 'Try the upload of BUILD=shipping (or dev) without writing: rclone, the secret file, the remote folders and the release notes are checked, the uploads run with --dry-run. With shipping, the tag is not required')
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
                                def dry = params.DEPLOY_DRY_RUN && uploadWanted() ? ' dry-run' : ''
                                currentBuild.displayName = "#${env.BUILD_NUMBER} v${env.APP_VERSION} ${buildKind()}${dry}"
                                // shipping: stop before building when a precondition is missing
                                if (isShipping()) {
                                    if (params.PLATFORM != 'both') {
                                        error("BUILD=shipping needs PLATFORM=both: a release has both installers")
                                    }
                                    if (!hasReleaseTag() && !params.DEPLOY_DRY_RUN) {
                                        error("BUILD=shipping: this commit is not tagged v${env.APP_VERSION} on the remote (see the release check above). " +
                                              "Make the release first: release.bat <version> push (macOS: ./release.sh <version> push), then build that commit")
                                    }
                                } else if (hasReleaseTag()) {
                                    echo "This commit has the tag v${env.APP_VERSION}, but BUILD=${buildKind()}: not a release (choose BUILD=shipping for that)"
                                }
                                // A dev upload must not fail the build, so only shipping requires rclone here
                                def rclone = (isShipping() && env.NODE == DEPLOY_NODE) ? '-DREQUIRE_RCLONE=ON' : ''
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
                            runCmd "cmake --build --preset ${platformOf(env.NODE)}-${configuration().toLowerCase()} --parallel"
                        }
                    }
                    stage('Test') {
                        when { expression { isImplemented(env.NODE) } }
                        steps {
                            // ctest writes the JUnit file relative to the build folder
                            runCmd "ctest --preset ${platformOf(env.NODE)}-${configuration().toLowerCase()} --output-junit ctest-junit.xml"
                        }
                        post {
                            always { junit testResults: "build/${platformOf(env.NODE)}/ctest-junit.xml", allowEmptyResults: true }
                        }
                    }
                    stage('Package') {
                        when { expression { isPackaged(env.NODE) && configuration() == 'Release' } }
                        steps {
                            dir('installer/output') { deleteDir() }   // installers of older versions must not be archived again
                            runCmd "cmake --build --preset ${platformOf(env.NODE)}-installer"
                            script {
                                if (uploadWanted()) {
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
                                def c = configuration()
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

        // Runs only when the selected platforms succeeded (a failed matrix stops the pipeline before this stage).
        // The Windows node uploads also for PLATFORM=macos (rclone is only there), so it has to be online
        stage('Deploy') {
            when { expression { uploadWanted() } }
            agent { label 'WindowsMini' }   // = DEPLOY_NODE (a field cannot be used in the agent label)
            steps {
                script {
                    def dry = params.DEPLOY_DRY_RUN ? '-DDRY_RUN=ON' : ''
                    def remote = "${DRIVE_REMOTE}:${DRIVE_FOLDER}"
                    if (isShipping()) {
                        if (!params.DEPLOY_DRY_RUN) {
                            // The tag must still point to this commit when the upload starts
                            runCmd 'cmake -DOUT=build/release_check.txt -P jenkins/scripts/release_check.cmake'
                            if (readFile('build/release_check.txt').trim() != 'yes') {
                                error("v${env.APP_VERSION}: the tag no longer points to this commit; nothing uploaded")
                            }
                        }
                        dir('installer/output') { deleteDir() }
                        packagedSelected().each { unstash "installer-${it}" }
                        withCredentials([file(credentialsId: RCLONE_CREDENTIAL, variable: 'RCLONE_CONFIG')]) {
                            runCmd "cmake -DCHANNEL=shipping -DREMOTE=${remote} -DFILES=installer/output -DWORK=build/deploy ${dry} -P jenkins/scripts/deploy.cmake"
                        }
                        if (!params.DEPLOY_DRY_RUN) {
                            currentBuild.description = "Shipped v${env.APP_VERSION}: ${remote}/v${env.APP_VERSION}/ and latest/"
                        }
                    } else {
                        // dev: a failed upload marks the build UNSTABLE; the build and its artifacts are fine
                        catchError(buildResult: 'UNSTABLE', stageResult: 'UNSTABLE',
                                   message: 'dev upload failed; the build itself passed (installers are in the Jenkins artifacts)') {
                            dir('installer/output') { deleteDir() }
                            packagedSelected().each { unstash "installer-${it}" }
                            withCredentials([file(credentialsId: RCLONE_CREDENTIAL, variable: 'RCLONE_CONFIG')]) {
                                runCmd "cmake -DCHANNEL=dev -DBUILD=${env.BUILD_NUMBER} -DKEEP=${DEV_KEEP} -DREMOTE=${remote} -DFILES=installer/output -DWORK=build/deploy ${dry} -P jenkins/scripts/deploy.cmake"
                            }
                            if (!params.DEPLOY_DRY_RUN) {
                                currentBuild.description = "dev: ${remote}/dev/${env.BUILD_NUMBER}-*/"
                            }
                        }
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
