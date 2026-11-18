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
// macOS: roadmap steps 7–8 (docs/CI_PLAN.md §10)
@Field final List IMPLEMENTED = ['windows']

String platformOf(String node) { return PLATFORMS[node] }
boolean isImplemented(String node) { return IMPLEMENTED.contains(platformOf(node)) }

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
echo "Install CMake 3.25+ (e.g. brew install cmake), then add its folder to the node's PATH:"
echo "  Manage Jenkins > Nodes > this node > Configure > Environment variables: PATH+CMAKE=/opt/homebrew/bin"
echo "Agent PATH: $PATH"
exit 1
'''
    } else {
        bat '''@echo off
where cmake >nul 2>nul && exit /b 0
echo ERROR: cmake is not in the PATH of the Jenkins agent on this node.
echo Install CMake 3.25+ with "Add CMake to the system PATH for all users", then restart the Jenkins agent,
echo or set PATH+CMAKE in the node settings: Manage Jenkins - Nodes - this node - Configure - Environment variables.
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
               description: 'Platform to build. macOS runs only the node check until its build exists (roadmap 7-8)')
        choice(name: 'CONFIGURATION', choices: ['Release', 'Debug'],
               description: 'Release also builds the installer. Debug: build and smoke test only')
        booleanParam(name: 'CLEAN_BUILD', defaultValue: false,
               description: 'Delete build/ first (rebuilds wxWidgets and PDF-Writer, a few minutes longer)')
        booleanParam(name: 'DEPLOY', defaultValue: true,
               description: 'STUB: prints a message only. Upload to Google Drive comes with roadmap 10 (set the default to false then)')
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
                            script {
                                env.APP_VERSION = readFile('VERSION').trim()
                                currentBuild.displayName = "#${env.BUILD_NUMBER} v${env.APP_VERSION} ${params.CONFIGURATION}"
                            }
                            requireCmake()
                            runCmd "cmake -DPLATFORM=${platformOf(env.NODE)} \"-DWORKSPACE=${env.WORKSPACE}\" -P jenkins/scripts/preflight.cmake"
                        }
                    }
                    stage('Not implemented') {
                        when { expression { !isImplemented(env.NODE) } }
                        steps {
                            echo "${platformOf(env.NODE)}: node check passed; the build itself is not implemented yet (docs/CI_PLAN.md roadmap 7-8)"
                        }
                    }
                    stage('Clean') {
                        when { expression { isImplemented(env.NODE) && params.CLEAN_BUILD } }
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
                        when { expression { isImplemented(env.NODE) && params.CONFIGURATION == 'Release' } }
                        steps {
                            dir('installer/output') { deleteDir() }   // installers of older versions must not be archived again
                            runCmd "cmake --build --preset ${platformOf(env.NODE)}-installer"
                        }
                    }
                    stage('Archive') {
                        when { expression { isImplemented(env.NODE) } }
                        steps {
                            script {
                                def p = platformOf(env.NODE)
                                def c = params.CONFIGURATION
                                def files = ["build/${p}/tests/out/${c}/*.pdf"]       // smoke test PDFs
                                if (c == 'Release') {
                                    files += ['installer/output/*', "build/${p}/bin/Release/*.pdb"]   // installer + PDBs for crash analysis
                                }
                                archiveArtifacts artifacts: files.join(', '), allowEmptyArchive: true, fingerprint: true
                            }
                        }
                    }
                }
            }
        }

        stage('Deploy') {
            when { expression { params.DEPLOY } }
            steps {
                echo 'DEPLOY is a stub: nothing is uploaded. Google Drive upload comes with roadmap 10 (docs/CI_PLAN.md); it will require CONFIGURATION=Release and a clean build'
            }
        }
    }

    post {
        failure { notifyResult('FAILURE') }
        fixed   { notifyResult('FIXED') }
    }
}
