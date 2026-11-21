# Completed Features

Moved out of CLAUDE.md on 2026-11-21 so it is not loaded into every Claude Code session; CLAUDE.md links here.
Add new completed items here (newest details also go to CHANGELOG.md `[미배포]` and docs/CI_PLAN.md §10).

- ✅ Full CRUD operations with Observer pattern
- ✅ SQLite storage provider with DI pattern
- ✅ File menu (New/Open/Save/SaveAs) with keyboard shortcuts
- ✅ Transaction filtering (date range, type, category) with Observer pattern integration
- ✅ UTF-8 encoding support for Korean text
- ✅ MVC architecture with strict boundaries (View → Controller → Model)
- ✅ List footer with income/expense/balance totals (real-time summary with filter support)
- ✅ Exchange rate support (foreign currency transactions with auto-conversion)
- ✅ Entity-DTO architecture refactoring (Data-Oriented Design, -136 lines net reduction)
- ✅ Budget system Phase 1-2: Complete budget management
  - Budget data structure with Type field and hierarchical Category/Item
  - SQLite budgets table with AUTOINCREMENT
  - Model/Controller CRUD methods with Observer pattern
  - 2-panel UI layout (Category list + Item detail list)
  - Inline input fields with Add/Update/Delete operations
  - Budget summary panel (budget amount only, no real-time calculation)
  - Complete save/load/delete functionality
  - CLI support (budget list/add/update/delete)
  - Transaction.BudgetId and ActualAmount fields reserved for future use
- ✅ PDF export (PDF-Writer, Malgun Gothic embedded) — user-facing details in [CHANGELOG.md](CHANGELOG.md)
  - Settlement report (`Ctrl+E`, CLI `export settlement <file.ml> [--out <pdf>]`): budget vs actual per category, built from `FMLSettlmentData`
  - Transaction list (`Ctrl+T`, CLI `export transactions <file.ml> [--out <pdf>]`): Category → Item → Transaction with merged cells, wrapping, inline exchange rates, subtotals
  - Structure: `FMLPDFDocument` → `FMLPDFRow` → cell; `DrawRow()` advances Y and splits merged cells across page breaks
- ✅ Windows Installer (Inno Setup) — [docs/INSTALLER.md](docs/INSTALLER.md)
  - `.ml` file association, Start menu/desktop shortcuts, uninstall cleanup
  - Bundles `vc_redist.x64.exe` (installed only when missing/outdated); admin install only
  - `cmake --build --preset windows-installer` (or `build_installer.bat`) → `installer\output\MissionLedger-<version>-Setup.exe`
- ✅ Versioning (v1.0.0) — [docs/VERSIONING.md](docs/VERSIONING.md)
  - Root `VERSION` file is the single source of truth (CMake → `.rc`; `.iss`, bat read it directly)
  - `CHANGELOG.md` (Keep a Changelog), annotated tags `vX.Y.Z`
- ✅ Core folder separation: Core sources in `MissionLedgerCore/src/` (one project = one folder)
- ✅ CMake build (2026-11-18): generated IDE projects, vendored wxWidgets, `.sln`/`.vcxproj`/`Version.props` removed. Verified against the former MSBuild build (identical CLI output and PDFs except the document ID, same version info, GUI checked, installer builds)
- ✅ Jenkins pipeline (2026-11-19): `jenkins/Build.Jenkinsfile` (matrix WindowsMini/MacMini, macOS = node check only), `jenkins/scripts/` (preflight, node_check, node_setup). First successful build on Jenkins: WindowsMini built, tested and packaged, MacMini passed the node check (about 10 minutes, first build)
- ✅ Formatting fixes (2026-11-20, found through the macOS build): amounts, decimals and console columns from one locale-independent Core class `FMLFormat`; foreign amount → KRW from `FMLCurrency::ConvertToKRW` (rounded, was truncated); GUI edit form keeps the exchange rate's precision; receipt-number sort no longer crashes on Korean or very long numbers. CLI/PDF output for `sample.ml` unchanged (PDFs byte-identical except the document ID); GUI checked by hand. Items only recorded: TODO.md "포맷/표시 관련 남은 항목"
- ✅ CTest smoke tests, `installer` target and `windows-release` workflow preset (2026-11-18): 5 CLI tests (`sample.ml` totals and PDFs, missing file); the workflow stops before packaging when a test fails; verified from a clean `build/` (about 2.5 minutes)
- ✅ macOS app and packaging (2026-11-21, roadmap 7 steps ③–⑤): `MissionLedger.app` (Universal arm64 + x86_64, macOS 11.0+, `.ml` document type, `MacOpenFiles`), unsigned `.dmg` (`macos-installer` preset), `lipo`/`otool` checks, smoke tests on both halves; GUI itself not run yet (see Next Steps 5)
- ✅ CLI rework steps 1–3 (2026-11-21, [docs/CLI_DESIGN.md](CLI_DESIGN.md)): `new` refuses an existing file; one grammar `<command> <file.ml> [options]` from one command table; `info`/`show`/`update`/`delete`, `list` filters, strict values, exit codes 0/1/2; `--json` (nlohmann/json), `--yes`, `--dry-run`, `--no-subtitle`, `.bak` backup before each write. 12 smoke tests
- ✅ External change detection (2026-11-22, CLI_DESIGN step 7): the GUI notices when another program (CLI) changes the open file: silent reload when nothing is unsaved, otherwise asks; asks before saving over external changes. Save and load are single SQLite transactions (`SaveAll`/`LoadAll`), `busy_timeout` 5 s
- ✅ CLI interactive mode rewrite (2026-11-22, CLI_DESIGN step 4): same commands as argument mode through one `dispatch()`, the open file is implied, missing fields asked step by step (categories/items by number) with preview and confirm, `취소` cancels, quoted arguments, `save` removed. 14 smoke tests
- ✅ PDF subtitle toggle (2026-11-21): File menu "PDF에 파일 이름 부제 넣기" (remembered with `wxConfig`), CLI `--no-subtitle`
- ✅ Release and deployment (2026-11-21): `release.bat`/`release.sh` (version number is the only input), Jenkins uploads a tagged commit's installers to Google Drive `MissionLedger/v<VERSION>/` + `latest/`; tested with a dry run (Jenkins #15), first real release pending
