# dual-readout

Repository for GEANT4 simulation &amp; analysis of the dual-readout calorimeter.

## How to compile

```bash
git clone -b INCREDIBLE https://github.com/kyHwangs/dual-readout.git
cd dual-readout
source envset.sh
mkdir build install
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install
make -j4 install
```

## How to use

### 1. GEANT4 standalone particle gun

In install,

```bash
./bin/DRsim <run_macro> <filenumber> <filename>
```

generates, `<filename>_<filenumber>.root`

### 2. Using script generator for condor batch job

In install,

```bash
./bin/make_script <project_name>
cd <project_name>
condor_submit condor_submit.sub
```

To check the condor job's status,

```bash
condor_q: for checking the status
    eg)
        [kyhwang@cms01 250721_step6_JetPU]$ condor_q
        -- Schedd: cms01.knu.ac.kr : <155.230.23.71:9618?... @ 07/23/25 11:42:20
        OWNER   BATCH_NAME      SUBMITTED   DONE   RUN    IDLE  TOTAL JOB_IDS
        kyhwang 1174355        7/23 09:31    354     74     13    441 1174355.7-439

        Total for query: 87 jobs; 0 completed, 0 removed, 13 idle, 74 running, 0 held, 0 suspended
        Total for kyhwang: 87 jobs; 0 completed, 0 removed, 13 idle, 74 running, 0 held, 0 suspended
        Total for all users: 255 jobs; 0 completed, 0 removed, 13 idle, 242 running, 0 held, 0 suspended

condor_rm <JOB_ID>: for removing condor job
```

### 3. Analysis

In install

```bash
./bin/analysis <project_name> <x_axis_min> <x_axis_max> 
```

### How to change beam particle and its energy
After running ```./bin/make_script <project_name>``` in Install, macro file will be generated with ```./<project_name>/run_macro.mac```.
```
# run_macro.mac

/vis/disable
/run/numberOfThreads 1
/run/initialize
/run/verbose 1

/DRsim/generator/randy 3.
/DRsim/generator/randz 3.
/DRsim/generator/nTower 0

/gun/particle e-
/gun/energy 20 GeV

/run/beamOn 2
```

```/gun/particle <particle>```: beam particle (e-, pi+ and etc.)
```/gun/energy <value> <unit>```: beam particel energy



