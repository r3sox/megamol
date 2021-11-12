import argparse
import os
import os.path
import pathlib
from PIL import Image
from SSIM_PIL import compare_ssim
import subprocess

parser = argparse.ArgumentParser(usage="%(prog)s <DIRECTORY>", description="execute test scripts in DIRECTORY")
parser.add_argument('directories', nargs="*")
parser.add_argument('--generate-reference', action='count', help='Generate reference pngs instead of testing against them')
parser.add_argument('--generate-neutral-test', action='count', help='Generate a first basic test (.1) for all found projects. Supply the MegaMol base folder as argument.')
parser.add_argument('--force', action='count', help='force overwriting files')
args = parser.parse_args()

RESULT_NAME = 'result.png'
IMPORT_PREFIX = '--MM_TEST_IMPORT '
testresults = []
CAPTURE_STDOUT = True
CAPTURE_STDERR = False

ssim_threshold = 0.95
class TestResult:
    testfile: str
    passed: bool
    result: str

def test_to_output(entry_path):
    file_name_only, _ = os.path.splitext(entry_path)
    return file_name_only + ".png", file_name_only + ".stdout", file_name_only + ".stderr"

def compare_images(reference, result):
    reference_image = Image.open(reference)
    result_image = Image.open(result)
    ssim_score = compare_ssim(reference_image, result_image, GPU=False)
    return ssim_score

if not args.directories:
    print("need at least one input directory")
    exit(1)

if args.generate_neutral_test:
    for directory in args.directories:
        parent = os.path.abspath(os.path.join(directory, os.pardir))
        for subdir, dirs, files in os.walk(directory, topdown=True):
            relpath = os.path.relpath(subdir, parent)
            pp = list(pathlib.Path(relpath).parts)
            pp[0] = "tests"
            testfolder = pathlib.Path(os.sep.join(map(str,pp)))
            #print(f"I am in subdir {relpath} of dir {parent} and test files would go to {testfolder}")
            for file in files:
                #print (f"I got file {file} and subdir {subdir}")
                entry = os.path.join(subdir, file)
                if entry.endswith('.lua'):
                    name, _ = os.path.splitext(file)
                    out = os.path.join(parent, testfolder, name + ".1.lua")
                    #print(f"I would make {out} from {entry}")
                    outpath = os.path.join(parent, testfolder)
                    if not os.path.isdir(outpath):
                        #print(f"making directory {outpath}")
                        os.makedirs(outpath)
                    if not os.path.isfile(out) or args.force:
                        print(f"making neutral test {out}")
                        with open(out, "w") as outfile:
                            outfile.write(f"{IMPORT_PREFIX} {os.path.relpath(entry, os.path.dirname(out))}\n")
                            outfile.write('mmRenderNextFrame()\nmmScreenshot("result.png")\nmmQuit()\n')
    exit(0)

for directory in args.directories:
    for subdir, dirs, files in os.walk(directory, topdown=True):
        for file in files:
            entry = os.path.join(subdir, file)
            if entry.endswith('.lua'):
                with open(entry) as infile:
                    lines = infile.readlines()
                    deps = []
                    for line in lines:
                        if line.startswith(IMPORT_PREFIX):
                            dep = line.removeprefix(IMPORT_PREFIX).strip()
                            deps.append(os.path.abspath(os.path.join(directory, subdir, dep)))
                            #print(f"found test for {deps}: {entry}")
                    commandline = "megamol.exe --nogui " + ' '.join(deps) + ' ' + entry
                    print(f"would exec: {commandline}")
                    refname, stdoutname, stderrname = test_to_output(entry)
                    #print(f"would expect same result as {refname}, stdout {stdoutname}, stderr {stderrname}")
                    if os.path.isfile(RESULT_NAME):
                        os.remove(RESULT_NAME)
                    if CAPTURE_STDOUT and os.path.isfile(stdoutname):
                        os.remove(stdoutname)
                    if CAPTURE_STDERR and os.path.isfile(stderrname):
                        os.remove(stderrname)
                    print(f"running test {entry}... ", end='')
                    tr = TestResult()
                    tr.testfile=entry
                    tr.passed=True
                    compl = subprocess.run(commandline, capture_output=True, check=True)
                    if args.generate_reference:
                        try:
                            os.rename(RESULT_NAME, refname)
                            print('generated reference')
                        except OSError as exception:
                            print(f'could not move {RESULT_NAME} to {refname}: {exception}')
                    else:
                        if not os.path.isfile(RESULT_NAME):
                            print('failed')
                            tr.passed = False
                            tr.result = "no output generated"
                            testresults.append(tr)
                            continue
                        if not os.path.isfile(refname):
                            print('failed')
                            tr.passed = False
                            tr.result = "missing reference image"
                            testresults.append(tr)
                            continue
                        try:
                            ssim = compare_images(refname, RESULT_NAME)
                            if ssim > ssim_threshold:
                                print(f'passed ({ssim})')
                            else:
                                print(f'failed ({ssim})')
                                tr.passed = False
                                if CAPTURE_STDOUT:
                                    with open(stdoutname, "w") as outfile:
                                        outfile.write(compl.stdout)
                                if CAPTURE_STDERR:
                                    with open(stderrname, "w") as outfile:
                                        outfile.write(compl.stderr)

                            tr.result = f'SSIM = {ssim}'
                            testresults.append(tr)
                        except Exception as exception:
                            tr.result = exception
                            tr.passed = False
                            testresults.append(tr)
                            print(f'unexpected exception: {exception}')

if args.generate_reference:
    exit(0)
    
if len(testresults) > 0:
    print("\nSummary:")
    for tr in testresults:
        print(f'{tr.testfile}: {"passed" if tr.passed else "failed"} {tr.result}')
else:
    print("no tests found.")
