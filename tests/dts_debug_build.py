#!/usr/bin/env python3
"""Exercise debug packaging isolation with a fake SDK and real arm.sh sanitizer."""
import hashlib,json,os,pathlib,shutil,subprocess,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='dts-debug-build-test-') as directory:
    fixture=pathlib.Path(directory); repository=fixture/'repo';tools=repository/'tools';tools.mkdir(parents=True)
    for name in ('build-dts-debug.sh','arm.sh','env.sh','tizen-config.xml'):
        shutil.copy2(ROOT/'tools'/name,tools/name)
    (tools/'p2p-motor').mkdir();shutil.copy2(ROOT/'tools/p2p-motor/pasta.sh',tools/'p2p-motor/pasta.sh')
    (repository/'src').mkdir();shutil.copy2(ROOT/'src/app_id.h',repository/'src/app_id.h')
    app=repository/'deploy/app';(app/'art').mkdir(parents=True)
    metadata=json.loads((ROOT/'deploy/app/appinfo.json').read_text())
    (app/'appinfo.json').write_text(json.dumps(metadata,indent=2)+'\n')
    (app/'licencas').mkdir();(app/'licencas/p2p-avisos.txt').write_text('fixture P2P notices')
    (app/'nuvio-proto').write_bytes(b'production executable stays intact')
    (app/'art/addons.txt').write_text('person credential must not ship')
    (app/'art/public.png').write_bytes(b'public asset')
    (repository/'production.ipk').write_bytes(b'production IPK stays intact')
    (repository/'local.properties').write_text('NUVIO_SUPABASE_URL=https://fixture.invalid\nNUVIO_SUPABASE_ANON_KEY=fixture-public\nTV_LOGIN_WEB_BASE_URL=https://fixture.invalid/login\n')
    runtime=fixture/'sdk';runtime.write_text('''#!/usr/bin/env python3
import os,pathlib,sys
args=sys.argv[1:]; assert 'fixture-sdk' in args
mount=args[args.index('-v')+1]; work=pathlib.Path(mount.split(':/work')[0])
env=pathlib.Path(args[args.index('--env-file')+1]).read_text()
flags=next(x for x in args if x.startswith('NUVIO_EXTRA_CFLAGS='))
assert 'NUVIO_DTS_FFMPEG=1' in args
assert '-DNV_DTS_DEBUG' in flags
assert '-DNV_APP_ID="space.nuvio.native.legacy.dtsdebug"' in flags
assert not (work/'deploy/app/art/addons.txt').exists()
assert not (work/'deploy/app/nuvio-proto').exists()
(work/'nuvio-proto.arm').write_text(env+'\\nspace.nuvio.native.legacy.dtsdebug\\n')
lib=work/'deploy/app/lib';lib.mkdir()
for version in (3,4): (lib/f'dts-starfish-webos{version}.so').write_bytes(b'fixture adapter')
licenses=work/'deploy/app/licenses/dts';licenses.mkdir(parents=True)
for name in ('COPYING.LGPLv2.1','SOURCE.txt'): (licenses/name).write_text('fixture notice')
''');runtime.chmod(0o755)
    packager=fixture/'ares-package';packager.write_text('''#!/usr/bin/env python3
import io,json,pathlib,subprocess,sys,tarfile,tempfile
app=pathlib.Path(sys.argv[1]);meta=json.loads((app/'appinfo.json').read_text())
assert not (app/'art/addons.txt').exists()
with tempfile.TemporaryDirectory() as d:
 root=pathlib.Path(d);(root/'debian-binary').write_text('2.0\\n')
 with tarfile.open(root/'data.tar.gz','w:gz') as tar:tar.add(app,arcname='usr/palm/applications/'+meta['id'])
 with tarfile.open(root/'control.tar.gz','w:gz'):pass
 output=pathlib.Path.cwd()/(meta['id']+'_'+meta['version']+'_arm.ipk')
 subprocess.check_call(['ar','r',str(output),'debian-binary','control.tar.gz','data.tar.gz'],cwd=root,stdout=subprocess.DEVNULL)
''');packager.chmod(0o755)
    before={p:hashlib.sha256(p.read_bytes()).hexdigest() for p in (app/'nuvio-proto',app/'appinfo.json',app/'art/addons.txt',repository/'production.ipk')}
    env=os.environ.copy();env.update(NUVIO_ARES_PACKAGE=str(packager),NUVIO_CONTAINER_RUNTIME=str(runtime),NUVIO_DTS_DEBUG_SDK_IMAGE='fixture-sdk',NUVIO_P2P_MOTOR='none')
    subprocess.run(['bash',str(tools/'build-dts-debug.sh')],cwd=repository,env=env,check=True,stdout=subprocess.DEVNULL)
    packages=list((repository/'build/dts-debug').glob('*.ipk'));assert len(packages)==1
    manifest=json.loads(packages[0].with_suffix('.ipk.json').read_text())
    assert manifest['id']=='space.nuvio.native.legacy.dtsdebug'
    assert manifest['title']=='Nuvio Legacy DTS Debug' and manifest['version']==metadata['version']
    assert manifest['sha256']==hashlib.sha256(packages[0].read_bytes()).hexdigest()
    for path,digest in before.items():assert hashlib.sha256(path.read_bytes()).hexdigest()==digest
print('dts_debug_build: identity, sanitized staging, checksums and production isolation passed')
