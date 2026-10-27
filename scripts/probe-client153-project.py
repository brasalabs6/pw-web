#!/usr/bin/env python3
"""Inventory the pinned VC project and check or compile its sources with Emscripten."""
import argparse
import hashlib
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path
import subprocess
import xml.etree.ElementTree as ET


def resolve_windows(root, relative):
    current = root
    for component in relative.replace('\\', '/').split('/'):
        if component in ('', '.'):
            continue
        if component == '..':
            current = current.parent
            continue
        exact = current / component
        if exact.exists():
            current = exact
            continue
        matches = [p for p in current.iterdir() if p.name.casefold() == component.casefold()] if current.is_dir() else []
        if len(matches) > 1:
            raise ValueError(f'ambiguous Windows path: {relative}')
        current = matches[0] if matches else exact
    return current


def port_include_dirs(port_root):
    port_root = port_root.resolve()
    paths = [port_root / directory for directory in
             ('AngelicaCommon/Header', 'AngelicaFile/Header', 'Angelica3D/Header')]
    paths.append(port_root.parent / 'platform' / 'common')
    return paths


def object_command(command, object_dir, source_path):
    digest = hashlib.sha256(source_path.encode('utf-8')).hexdigest()
    return [part for part in command if part != '-fsyntax-only'] + [
        '-c', '-o', str(object_dir / (digest + '.o'))]


def first_error(stderr):
    for line in stderr.splitlines():
        for marker in ('fatal error: ', 'error: '):
            if marker in line:
                return marker + line.split(marker, 1)[1]
    return 'no compiler error diagnostic'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--emxx', required=True)
    parser.add_argument('--project', default='Element/CElementClient/ElementClient.vcproj')
    parser.add_argument('--configuration', default='Release|Win32')
    parser.add_argument('--entry', default='ElementClient.cpp')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--all', action='store_true', help='syntax-check every non-excluded source')
    parser.add_argument('--port-root', type=Path, help='prefer already ported Angelica headers')
    parser.add_argument('--adapted-root', type=Path, help='compile the adapted copy of a source when one exists under this root')
    parser.add_argument('--jobs', type=int, default=4)
    parser.add_argument('--object-dir', type=Path, help='compile actual WASM objects instead of syntax-only checks (requires --all)')
    args = parser.parse_args()
    if args.object_dir and not args.all:
        parser.error('--object-dir requires --all')
    if args.jobs < 1 or args.jobs > 16:
        parser.error('--jobs must be between 1 and 16')
    root = args.source.resolve()
    project = root / args.project
    tree = ET.fromstring(project.read_bytes().decode('gb18030'))
    config = next((c for c in tree.findall('./Configurations/Configuration') if c.attrib['Name'] == args.configuration), None)
    if config is None:
        parser.error('configuration not present in original project')
    tools = {t.attrib['Name']: t.attrib for t in config.findall('Tool')}
    cc = tools['VCCLCompilerTool']
    includes = [resolve_windows(project.parent, p) for p in cc['AdditionalIncludeDirectories'].split(';')]
    if args.adapted_root:
        #	Mirror every reference include dir that exists in the adapted
        #	tree, ahead of the reference, so include chains stay inside the
        #	adapted copies (prevents double-parsing the same header from both
        #	trees).
        mirrored = []
        for inc in includes:
            try:
                rel = inc.relative_to(root)
            except ValueError:
                continue
            adapted_inc = args.adapted_root.resolve() / rel
            if adapted_inc.is_dir():
                mirrored.append(adapted_inc)
        includes = mirrored + includes
    sources = []
    for entry in tree.findall('.//File'):
        path = resolve_windows(project.parent, entry.attrib['RelativePath'])
        if path.suffix.lower() not in ('.cpp', '.c', '.cxx'):
            continue
        excluded = any(x.attrib.get('ExcludedFromBuild') == 'true' for x in [x for x in entry.findall('./FileConfiguration') if x.attrib['Name'] == args.configuration])
        sources.append(dict(path=str(path.relative_to(root)), exists=path.is_file(), excluded=excluded))
    seam = (args.port_root.parent / 'platform' / 'common' / 'Windows.h') if args.port_root else None
    forced = ['-include', str(seam.resolve())] if seam and seam.is_file() else []
    #	The original build supplied the AUI widget types through precompiled
    #	headers; the SDK's AUIDialog.h uses AUIComboBox without including it.
    #	The widget chain compiles against the port; the D3D font headers stay
    #	out of this prelude (they belong to the renderer cycle).
    aui_dir = root / 'Common' / 'Output' / 'include' / 'aui'
    for widget in ('AUIListBox.h', 'AUIComboBox.h', 'AUIEditBox.h', 'AUILabel.h',
                   'AUIStillImageButton.h', 'AUICheckBox.h', 'AUIImagePicture.h',
                   'AUIDialog.h'):
        aui = aui_dir / widget
        if aui.is_file():
            forced += ['-include', str(aui)]
    sdk_forward = args.port_root.parent / 'platform' / 'common' / 'sdk_forward.h' if args.port_root else None
    if sdk_forward and sdk_forward.is_file():
        forced += ['-include', str(sdk_forward.resolve())]
    command = [args.emxx, '-fsyntax-only', '-ferror-limit=5', '-fms-extensions', '-std=c++17', '-fshort-wchar'] + forced + [str(resolve_windows(project.parent, args.entry))]
    if args.port_root:
        for path in port_include_dirs(args.port_root):
            if not path.is_dir():
                parser.error(f'port header directory missing: {path}')
            command += ['-I', str(path)]
    for path in includes:
        command += ['-I', str(path)]
    command += ['-D' + d for d in cc['PreprocessorDefinitions'].split(';') if d]
    result = subprocess.run(command, capture_output=True, text=True)
    report = dict(project=args.project, configuration=args.configuration, sources=sources,
                  includes=[dict(path=str(p.relative_to(root)) if p.is_relative_to(root) else str(p), exists=p.is_dir()) for p in includes],
                  definitions=cc['PreprocessorDefinitions'].split(';'),
                  libraries=tools.get('VCLinkerTool', tools.get('VCLibrarianTool', {})).get('AdditionalDependencies', '').split(),
                  probe=dict(command=command, exitCode=result.returncode, stderr=result.stderr))
    if args.all:
        if args.object_dir:
            args.object_dir.mkdir(parents=True, exist_ok=True)
        source_index = command.index(str(resolve_windows(project.parent, args.entry)))
        adapted_root = args.adapted_root.resolve() if args.adapted_root else None
        def compile_source(source):
            cmd = command.copy()
            source_path = root / source['path']
            if adapted_root:
                adapted = adapted_root / source['path']
                if adapted.is_file():
                    source_path = adapted
            cmd[source_index] = str(source_path)
            if args.object_dir:
                cmd = object_command(cmd, args.object_dir.resolve(), source['path'])
            try:
                outcome = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
                return dict(path=source['path'], exitCode=outcome.returncode,
                            firstError=first_error(outcome.stderr) if outcome.returncode else None)
            except subprocess.TimeoutExpired:
                return dict(path=source['path'], exitCode=-1, firstError='compiler timeout after 120 seconds')
        active = [source for source in sources if source['exists'] and not source['excluded']]
        with ThreadPoolExecutor(max_workers=args.jobs) as executor:
            outcomes = list(executor.map(compile_source, active))
        groups = Counter(x['firstError'] for x in outcomes if x['firstError'])
        report['buildMode'] = 'wasm-objects' if args.object_dir else 'syntax-only'
        report['allSources'] = outcomes
        report['errorGroups'] = dict(groups.most_common())
        print(f"Checked {len(outcomes)} sources; {sum(x['exitCode'] == 0 for x in outcomes)} passed")
        for message, count in groups.most_common(10):
            print(f'{count:4d} {message}')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    print(f'{len(sources)} translation units; {sum(not s["exists"] for s in sources)} missing after Windows case resolution')
    print(result.stderr)
    return 1 if args.all and any(x['exitCode'] for x in outcomes) else result.returncode


if __name__ == '__main__':
    raise SystemExit(main())
