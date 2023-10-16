#!/usr/bin/env python3
import subprocess
import os

def run_stress_test(
    malloc_conf,
    batch_size = 1000,
    batch_frees = 200,
    num_threads = 256,
    num_runs = 5000,
    malloc_size = 32):
    # The default setting takes ~2min for each test.
    env = os.environ.copy()
    env['MALLOC_CONF'] = malloc_conf
    print(f'MALLOC_CONF={malloc_conf}, '
          f'batch_size={batch_size}, batch_frees={batch_frees}, '
          f'num_threads={num_threads}, num_runs={num_runs}, '
          f'malloc_size={malloc_size} ',
          flush=True)
    process = subprocess.Popen(
        [
            './fillFlush',
            f'--batch_size={batch_size}',
            f'--batch_frees={batch_frees}',
            f'--num_threads={num_threads}',
            f'--num_runs={num_runs}',
            f'--malloc_size={malloc_size}'
        ],
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )
    stdout, stderr = process.communicate()
    print(f'Return code: {process.returncode}', flush=True)
    print(f'Standard Output:\n{stdout.decode("utf-8")}', flush=True)
    print(f'Standard Error:\n{stderr.decode("utf-8")}', flush=True)
    print('-' * 80, flush=True)

    return

def main():
    malloc_configs = [
        'tcache:false,prof:false',
        'tcache:false,prof:true,lg_prof_sample:13',
        'tcache:true,prof:false,lg_prof_sample:13,ncached_max:1-1024:100|1025-2048:1|2049-1000000:0,tcache_max:4096',
        'tcache:true,prof:true,lg_prof_sample:13,ncached_max:1-1024:100|1025-99999:1|2049-1000000:0,tcache_max:4096',
        'tcache:true,prof:true,lg_prof_sample:13,ncached_max:1-1024:100|1025-99999:1|2049-1000000:0,tcache_max:4096,percpu_arena:percpu'
    ]

    for config in malloc_configs:
        run_stress_test(config, num_threads=500)
        run_stress_test(config, malloc_size=2000, batch_size=100,batch_frees=100, num_threads=500)
        run_stress_test(config, malloc_size=5000, batch_size=100,batch_frees=100, num_threads=500)

if __name__ == '__main__':
    main()

