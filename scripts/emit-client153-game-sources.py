#!/usr/bin/env python3
"""Emit the full original game source list for the reproducible CMake build.

Reads the pinned .vcproj files (the build system the original client
actually used) and writes a CMake fragment with the non-excluded C++
sources mapped into the adapted tree, plus the include directories the
probe derives from the projects. The CMakeLists includes the fragment to
compile the complete game in one target — replacing the campaign-object
assembly that mixed stale objects.

Usage: emit-client153-game-sources.py --source <reference root> \
         --adapted-root <src/client153> --output <game-sources.cmake>
"""

import argparse
import pathlib
import sys
import xml.etree.ElementTree as ET

ROOT = pathlib.Path(__file__).resolve().parents[1]

PROJECTS = (
    'Element/CElementClient/ElementClient.vcproj',
    'Common/AUInterface2/AUInterface_2.2.vcproj',
)
CONFIGURATION = 'Release|Win32'

# The Windows-native anti-cheat TUs (MS inline asm + SEH) cannot execute in
# a browser sandbox. The port's defence SEAM (defence.cpp, imagechecker.cpp)
# stays in the build and fails explicitly at runtime; only these native
# implementations are excluded.
BROWSER_EXCLUDED_SOURCES = (
    'Element/CElementClient/defence/APILoader.cpp',
    'Element/CElementClient/defence/cheaterkiller.cpp',
    'Element/CElementClient/defence/pwacp.cpp',
    'Element/CElementClient/defence/stackchecker.cpp',
)

# Sources the original .vcproj files omit but the port's link requires (the
# CMake game target adds them manually; the adapted tree carries them).
EXTRA_SOURCES = (
    'Element/CFWEditorLib/Mesh.cpp',
    'Element/CFWEditorLib/FWTextMesh.cpp',
    'Element/CFWEditorLib/Triangle.cpp',
    'Element/CFWEditorLib/CharContour.cpp',
    'Element/CFWEditorLib/CharCurve.cpp',
    'Element/CFWEditorLib/CharOutline.cpp',
    'Element/CFWEditorLib/CharVerticesBuffer.cpp',
    'Element/CFWEditorLib/GLUPolygon.cpp',
    'Element/CFWEditorLib/GLUTess.cpp',
    'Element/CFWEditorLib/GLUContour.cpp',
    'Element/CFWEditorLib/FWFormula.cpp',
    'Element/CFWEditorLib/FWFormulaElement.cpp',
    'Element/CFWEditorLib/FWOperand.cpp',
    'Element/CFWEditorLib/FWOperator.cpp',
    'Element/CFWEditorLib/FWArt.cpp',
    'Element/CFWEditorLib/FWModifier.cpp',
    'Element/CFWEditorLib/FWArtArc.cpp',
    'Element/CFWEditorLib/FWArtDoubleArc.cpp',
    'Element/CFWEditorLib/FWArtNoChange.cpp',
    'Element/CFWEditorLib/FWArtStepScale.cpp',
    'Element/CFWEditorLib/FWArtTilt.cpp',
    'Element/CFWEditorLib/FWArtWave.cpp',
    'Element/CFWEditorLib/FWModifierParabola.cpp',
    'Element/CFWEditorLib/FWModifierRing.cpp',
    'Element/CFWEditorLib/FWObject.cpp',
    'Element/CFWEditorLib/FWGlyphGraphBase.cpp',
    'Element/CFWEditorLib/FWGlyphPolygon.cpp',
    'Element/CFWEditorLib/FWGlyphBezier.cpp',
    'Element/CFWEditorLib/FWPlayer.cpp',
    'Element/CFWEditorLib/FWState.cpp',
    'Element/CFWEditorLib/FWDialogBase.cpp',
    'Element/CFWEditorLib/FWModifierCreator.cpp',
    'Element/CFWEditorLib/FWArtCreator.cpp',
    'Element/CFWEditorLib/FWFireworkCustom.cpp',
    'Element/CFWEditorLib/FWFireworkGraph.cpp',
    'Element/CFWEditorLib/FWFireworkSimple.cpp',
    'Element/CFWEditorLib/FWFireworkElement.cpp',
    'Element/CFWEditorLib/FWLog.cpp',
    'Element/CElementSkill/ElementSkill.cpp',
    'Element/CElementSkill/skill.cpp',
    'Element/CElementSkill/skillwrapper.cpp',
    'Element/AutoPFImp/AutoMoveImp/MoveAgent.cpp',
    'Element/AutoPFImp/AutoMoveImp/PfClu.cpp',
    'Element/AutoPFImp/AutoMoveImp/PfConstant.cpp',
    'Element/AutoPFImp/AutoMoveImp/MoveMap.cpp',
    'Element/AutoPFImp/AutoMoveImp/LayerMap.cpp',
    'Element/AutoPFImp/AutoMoveImp/Island.cpp',
    'Element/AutoPFImp/AutoMoveImp/OptimizePath.cpp',
    'Element/AutoPFImp/AutoMoveImp/Graph.cpp',
    'Element/AutoPFImp/AutoMoveImp/GHeap.cpp',
    'Element/AutoPFImp/AutoMoveImp/MapNode.cpp',
    'Element/AutoPFImp/AutoMoveImp/MapGraph.cpp',
    'Element/AutoPFImp/AutoMoveImp/GAStar.cpp',
    'Element/AutoPFImp/AutoMoveImp/MAStar.cpp',
    'Element/AutoPFImp/AutoMoveImp/CluCommon.cpp',
    'Element/AutoPFImp/AutoMoveImp/CluGraph.cpp',
    'Element/AutoPFImp/AutoMoveImp/ClusterAbstraction.cpp',
    'Element/AutoPFImp/AutoMoveImp/CluGraphConnector.cpp',
    'Element/AutoPFImp/AutoMoveImp/MultiCluGraph.cpp',
    'Common/CHBasedCD/ConvexHullData.cpp',
    'Common/CHBasedCD/aabbcd.cpp',
    'Common/CHBasedCD/HalfSpace.cpp',
    'Common/CHBasedCD/CDWithCH.cpp',
    'Common/CHBasedCD/Face.cpp',
    'Common/CHBasedCD/Matrix.cpp',
    'Common/CHBasedCD/LEquations.cpp',
    'Common/CHBasedCD/BestFit.cpp',
    'Element/CFWEditorLib/FWAUIManager.cpp',
    'Element/CFWEditorLib/FWAlgorithm.cpp',
    'Element/CFWEditorLib/FWArchive.cpp',
    'Element/CFWEditorLib/FWAssemblyCache.cpp',
    'Element/CFWEditorLib/FWAssemblyProfile.cpp',
    'Element/CFWEditorLib/FWAssemblySet.cpp',
    'Element/CFWEditorLib/FWCommandList.cpp',
    'Element/CFWEditorLib/FWCommandSetGlyphProp.cpp',
    'Element/CFWEditorLib/FWDialogBezierMenu.cpp',
    'Element/CFWEditorLib/FWDialogBoard.cpp',
    'Element/CFWEditorLib/FWDialogCustom.cpp',
    'Element/CFWEditorLib/FWDialogEdit.cpp',
    'Element/CFWEditorLib/FWDialogGraph.cpp',
    'Element/CFWEditorLib/FWDialogGraphMenu.cpp',
    'Element/CFWEditorLib/FWDialogLanch.cpp',
    'Element/CFWEditorLib/FWDialogMain.cpp',
    'Element/CFWEditorLib/FWDialogParam.cpp',
    'Element/CFWEditorLib/FWDialogPlay.cpp',
    'Element/CFWEditorLib/FWDialogPolygonMenu.cpp',
    'Element/CFWEditorLib/FWDialogScale.cpp',
    'Element/CFWEditorLib/FWDialogSimple.cpp',
    'Element/CFWEditorLib/FWDialogText.cpp',
    'Element/CFWEditorLib/FWDialogTool.cpp',
    'Element/CFWEditorLib/FWDoc.cpp',
    'Element/CFWEditorLib/FWFirework.cpp',
    'Element/CFWEditorLib/FWFireworkLanch.cpp',
    'Element/CFWEditorLib/FWFireworkLeaf.cpp',
    'Element/CFWEditorLib/FWFlatCollector.cpp',
    'Element/CFWEditorLib/FWFontEnum.cpp',
    'Element/CFWEditorLib/FWFormulaList.cpp',
    'Element/CFWEditorLib/FWGfxWithModifier.cpp',
    'Element/CFWEditorLib/FWGlyph.cpp',
    'Element/CFWEditorLib/FWGlyphComposite.cpp',
    'Element/CFWEditorLib/FWGlyphFirework.cpp',
    'Element/CFWEditorLib/FWGlyphList.cpp',
    'Element/CFWEditorLib/FWGlyphRectBase.cpp',
    'Element/CFWEditorLib/FWGlyphText.cpp',
    'Element/CFWEditorLib/FWInnerProperty.cpp',
    'Element/CFWEditorLib/FWManager.cpp',
    'Element/CFWEditorLib/FWOutterProperty.cpp',
    'Element/CFWEditorLib/FWRuntimeClass.cpp',
    'Element/CFWEditorLib/FWStateSelect.cpp',
    'Element/CFWEditorLib/FWStream.cpp',
    'Element/CFWEditorLib/FWTemplate.cpp',
    'Element/CFWEditorLib/FWView.cpp',
    'Common/Output/source/Gfx/RandStringContainer.cpp',
)


def resolve_windows(base: pathlib.Path, relative: str) -> pathlib.Path:
    """Resolve a Windows-style relative path against a project directory,
    with a case-insensitive fallback per segment (the original build ran on
    a case-insensitive file system; the pinned checkout is case-sensitive)."""
    clean = relative.replace('\\', '/').strip()
    path = base / clean
    if path.exists():
        return path.resolve() if path.exists() else path
    # Walk the segments, matching each case-insensitively.
    current = base
    for segment in clean.split('/'):
        if not segment or segment == '.':
            continue
        if segment == '..':
            current = current.parent
            continue
        candidate = current / segment
        if candidate.exists():
            current = candidate
            continue
        try:
            match = next((entry for entry in current.iterdir()
                          if entry.name.lower() == segment.lower()), None)
        except OSError:
            match = None
        if match is None:
            return path  # unresolvable: report the literal path
        current = match
    return current


def split_includes(value: str):
    return [item.strip() for item in __import__('re').split(r'[;,]', value) if item.strip()]


def portable_include_dirs(root: pathlib.Path, includes):
    windows_zlib = root / 'Angelica2' / 'Output' / '3rdsdk' / 'include' / 'zlib'
    return [path for path in includes if path != windows_zlib]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=pathlib.Path, required=True,
                        help='the pinned reference checkout (references/client-153)')
    parser.add_argument('--adapted-root', type=pathlib.Path, required=True,
                        help='the adapted tree (src/client153)')
    parser.add_argument('--output', type=pathlib.Path, required=True,
                        help='the CMake fragment to write')
    args = parser.parse_args()

    root = args.source.resolve()
    adapted = args.adapted_root.resolve()
    repo_root = adapted.parents[1]  # the checkout root (src/client153's parent's parent... src's parent)

    def portable(path: pathlib.Path) -> str:
        """Map a path to ${PW153_REPO_ROOT}/... (the adapted tree) or
        ${PW153_REFERENCE_ROOT}/... (the pinned reference) — the fragment
        must not contain workstation-absolute paths (the CI configure
        failure). The mapping is decided by which tree owns the path, not
        by generic prefix matching: the CI materializes the reference
        INSIDE the checkout (references/client-153), and those files must
        still map to the overridable reference root — the two roots are
        distinct contracts."""
        if adapted == path or adapted in path.parents:
            return '${PW153_REPO_ROOT}/' + str(path.relative_to(repo_root))
        if root == path or root in path.parents:
            return '${PW153_REFERENCE_ROOT}/' + str(path.relative_to(root))
        raise SystemExit(f'path outside both roots: {path}')

    sources = []
    include_dirs = []
    # U02: a TU the original build compiles that neither tree carries is a
    # broken manifest, not a skippable entry — collected here and turned
    # into a hard error with the complete inventory at the end. The only
    # allowed omissions are the explicit ones: ExcludedFromBuild in the
    # .vcproj and BROWSER_EXCLUDED_SOURCES (both are declarations, not
    # silent gaps).
    missing = []
    applied_exclusions = set()
    defines = []

    # The port seam headers come first: Windows.h's own includes (ABaseDef.h
    # etc.) must resolve to the PORT's copies — a reference ABaseDef defines
    # LONG as `long`, collapsing the LONG/long InterlockedExchange overloads.
    include_dirs += [portable(adapted / d) for d in
                     ('platform/common', 'Angelica2/AngelicaCommon/Header',
                      'Angelica2/AngelicaFile/Header', 'Angelica2/Angelica3D/Header',
                      'Common/AUInterface2')]
    include_dirs.append(portable(root / 'Common' / 'CHBasedCD'))

    for project_rel in PROJECTS:
        project = root / project_rel
        if not project.is_file():
            raise SystemExit(f'missing project file: {project} — check out the pinned reference')
        tree = ET.fromstring(project.read_bytes().decode('gb18030'))
        config = next((c for c in tree.findall('./Configurations/Configuration')
                       if c.attrib['Name'] == CONFIGURATION), None)
        if config is None:
            raise SystemExit(f'configuration {CONFIGURATION} not present in {project}')
        tools = {t.attrib['Name']: t.attrib for t in config.findall('Tool')}
        cc = tools['VCCLCompilerTool']
        for define in split_includes(cc['PreprocessorDefinitions']):
            defines.append(define)
        for inc in split_includes(cc['AdditionalIncludeDirectories']):
            resolved = resolve_windows(project.parent, inc)
            try:
                rel = resolved.relative_to(root)
            except ValueError:
                continue
            rel_str = str(rel).replace('\\', '/')
            adapted_inc = adapted / rel
            is_sdk_dir = 'angelica2' in rel_str.lower()
            if adapted_inc.is_dir():
                # The adapted copy first: headers that were never copied to
                # the adapted tree (callid.hxx etc.) still resolve from the
                # pinned reference checkout — EXCEPT the SDK dirs, whose
                # reference headers are the raw Windows ones and conflict
                # with the port's Windows.h (InterlockedExchange etc.).
                include_dirs.append(portable(adapted_inc))
                if not is_sdk_dir:
                    include_dirs.append(portable(root / rel))
            else:
                include_dirs.append(portable(root / rel))
        for entry in tree.findall('.//File'):
            path = resolve_windows(project.parent, entry.attrib['RelativePath'])
            if path.suffix.lower() not in ('.cpp', '.c', '.cxx'):
                continue
            excluded = any(x.attrib.get('ExcludedFromBuild') == 'true'
                           for x in entry.findall('./FileConfiguration')
                           if x.attrib['Name'] == CONFIGURATION)
            if excluded:
                continue
            try:
                rel = path.relative_to(root)
            except ValueError:
                continue
            rel_str = str(rel).replace('\\', '/')
            if rel_str in BROWSER_EXCLUDED_SOURCES:
                applied_exclusions.add(rel_str)
                continue
            # The adapted copy wins when it exists (the port fixes live there).
            adapted_source = adapted / rel
            reference_source = root / rel
            if adapted_source.is_file():
                sources.append(portable(adapted_source))
            elif reference_source.is_file():
                sources.append(portable(reference_source))
            else:
                missing.append(f'vcproj source absent from both trees: {rel_str}')

    # The port seam headers come first (the prelude chain lives there).
    # The collision-headers were never copied to the adapted CHBasedCD (only
    # its .cpp files were); the pinned reference carries the complete set.
    include_dirs += [f'${{PW153_REPO_ROOT}}/{d}' for d in
                     ('Element/Output/include', 'Element/CCommon',
                      'Element/CElementClient', 'Element/Output/include/fw',
                      'Element/CElementSkill', 'Element/AutoPFImp/AutoMoveImp',
                      'Common/CHBasedCD', 'Common/AUInterface2',
                      'Common/Output/include', 'Common/Output/include/Gfx',
                      'Common/Output/include/PropDlg', 'Common/Output/include/LuaWrapper',
                      'Common/Output/include/aaudio', 'Element/CCommon/lua')]

    # Deduplicate, preserving order.
    seen = set()
    sources = [s for s in sources if not (s in seen or seen.add(s))]
    # The vcproj-omitted sources the port's link requires.
    for extra in EXTRA_SOURCES:
        adapted_extra = adapted / extra
        reference_extra = root / extra
        if adapted_extra.is_file():
            portable_extra = portable(adapted_extra)
            if portable_extra not in seen:
                sources.append(portable_extra)
                seen.add(portable_extra)
        elif reference_extra.is_file():
            portable_extra = portable(reference_extra)
            if portable_extra not in seen:
                sources.append(portable_extra)
                seen.add(portable_extra)
        else:
            missing.append(f'link-required extra source absent from both trees: {extra}')
    # An exclusion entry that matched no .vcproj file is a stale
    # declaration: the allowlist must stay exact, or the manifest silently
    # narrows (U02 — explicit, verified exclusions only).
    stale_exclusions = [entry for entry in BROWSER_EXCLUDED_SOURCES
                        if entry not in applied_exclusions]
    if missing or stale_exclusions:
        for entry in missing:
            print(f'error: {entry}', file=sys.stderr)
        for entry in stale_exclusions:
            print(f'error: BROWSER_EXCLUDED_SOURCES entry matched no project file '
                  f'(stale exclusion): {entry}', file=sys.stderr)
        print(f'{len(missing)} mandatory source(s) missing and '
              f'{len(stale_exclusions)} stale exclusion(s) — the manifest must '
              'declare every entry and omit nothing silently (U02)',
              file=sys.stderr)
        raise SystemExit(1)
    seen = set()
    include_dirs = [d for d in include_dirs if not (d in seen or seen.add(d))]
    seen = set()
    defines = [d for d in defines if not (d in seen or seen.add(d))]

    lines = [
        '# GENERATED FILE - DO NOT EDIT.',
        '# Generated by scripts/emit-client153-game-sources.py from the pinned',
        '# .vcproj files (the original build system). Regenerate after any',
        '# change to the reference projects.',
        'set(CLIENT153_GAME_SOURCES)',
    ]
    for source in sources:
        lines.append(f'  list(APPEND CLIENT153_GAME_SOURCES "{source}")')
    lines.append('set(CLIENT153_GAME_INCLUDE_DIRS)')
    for inc in include_dirs:
        lines.append(f'  list(APPEND CLIENT153_GAME_INCLUDE_DIRS "{inc}")')
    lines.append('set(CLIENT153_GAME_COMPILE_DEFINITIONS)')
    for define in defines:
        lines.append(f'  list(APPEND CLIENT153_GAME_COMPILE_DEFINITIONS "{define}")')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text('\n'.join(lines) + '\n', encoding='utf-8')
    print(f'sources: {len(sources)}  include dirs: {len(include_dirs)}  '
          f'exclusions: {len(applied_exclusions)}  wrote {args.output}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
