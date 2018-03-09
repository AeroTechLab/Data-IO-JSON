# Data I/O JSON
[JSON](https://www.json.org/json-pt.html)-based implementation of [Data I/O Interface](https://github.com/LabDin/Data-IO-Interface)

## Usage

On a terminal, get the [GitHub code repository](https://github.com/LabDin/Data-IO-JSON) with:

    $ git clone https://github.com/LabDin/Data-IO-JSON [<my_project_folder>]

This implementation depends on [Data I/O Interface](https://github.com/LabDin/Data-IO-Interface) and [Simple JSON](https://github.com/LabDin/Simple-JSON) projects, which are added as [git submodules](https://git-scm.com/docs/git-submodule).

To add those repositories to your sources, navigate to the root project folder and clone them with:

    $ cd <my_project_folder>
    $ git submodule update --init

With dependencies set, you can now build the library to a separate build directory with [CMake](https://cmake.org/):

    $ mkdir build && cd build
    $ cmake .. 
    $ make

For building it manually e.g. with [GCC](https://gcc.gnu.org/) in a system without **CMake** available, the following shell command (from project directory) would be required:

    $ gcc data_io_json.c json/json.c -I/interface -I/json -shared -fPIC -o libdataiojson.{so,dll}
