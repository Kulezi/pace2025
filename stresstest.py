#!/bin/python3
import subprocess
import sys
from tqdm import tqdm

def run_command(command, input_file=None, output_file=None, error_file=None):
    """Uruchamia polecenie i obsługuje błędy."""
    infile = open(input_file, 'r') if input_file else None
    outfile = open(output_file, 'w') if output_file else None
    errfile = open(error_file, 'w') if error_file else None
    
    try:
        result = subprocess.run(command, stdin=infile, stdout=outfile, stderr=errfile)
    finally:
        if infile:
            infile.close()
        if outfile:
            outfile.close()
        if errfile:
            errfile.close()
    
    return result.returncode

def main():
    # Kompilacja
    subprocess.run(["cmake", "."], check=True)
    subprocess.run(["make"], check=True)
    
    for i in tqdm(range(1, 10001), desc="Stress test progress"):
        if run_command(["./random_graph_gen.out", str(i)], output_file="rg.gr") != 0:
            print("Runtime error in rg.out")
            sys.exit(1)

        if run_command(["./main.out", "--method", "bruteforce", "--presolve", "none"], input_file="rg.gr", output_file="brute.sol") != 0:
            print("Runtime error in brute.out")
            with open("rg.gr") as f:
                print(f.read())
            sys.exit(1)
        
        if run_command(["./main.out"], input_file="rg.gr", output_file="main.sol", error_file="main.log") != 0:
            print("Runtime error in main.out")
            with open("rg.gr") as f:
                print(f.read())
            with open("main.log") as f:
                print(f.read())
            sys.exit(1)
        
        with open("main.sol") as f:
            main_count = f.readline().strip()
        with open("brute.sol") as f:
            brute_count = f.readline().strip()
        
        if main_count != brute_count:
            print("Mismatch found!")
            with open("rg.gr") as f:
                print(f.read())
            with open("main.log") as f:
                print(f.read())
            sys.exit(1)
    
    print("[OK] Stress test done")
    subprocess.run(["make", "clean"], check=True)

if __name__ == "__main__":
    main()