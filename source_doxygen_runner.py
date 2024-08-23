import shutil
import subprocess
import tempfile
from pathlib import Path

if __name__ == '__main__':
    root_path = Path(__file__).resolve().parent
    # Copy doxyfile and add INPUT override (https://stackoverflow.com/a/11038467). Doxygen must be run this way because meson sets the
    # working directory to the build dir, which doesn't contain source code. There is no easy way to set the work directory via command line
    # and so this script is the next best optoin.
    doxygen_config_path = root_path / 'doxyfile'
    doxygen_config_temp_path = tempfile.NamedTemporaryFile(delete=True)
    shutil.copyfile(doxygen_config_path, doxygen_config_temp_path.name)
    with open(doxygen_config_temp_path.name, 'a') as f:
        f.writelines([
            f'INPUT = {root_path / 'offbynull'}\n'
        ],)
    result = subprocess.run(['doxygen', doxygen_config_temp_path.name], shell=False, stdout=None, stderr=None)
    exit(result.returncode)
