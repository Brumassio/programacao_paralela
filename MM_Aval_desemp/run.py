import subprocess
import os


sizes = [1000, 2000, 4000]
cores = [2, 4, 8, 16]


def main():
    nome = input("Digite o nome da pasta que deseja salvar o log: ")
    if not os.path.exists(nome+'/'):
        os.mkdir(nome)
    for size in sizes:
        cmd = f"python3 script.py {size} {0}"
        cmd = cmd + f" >> {nome}/log_{size}_seq.txt"
        print(f"Running: '{cmd}' and saving at 'log_{size}_seq.txt'")
        subprocess.call(cmd, shell=True)

    for size in sizes:
        for core in cores:
            cmd = f"python3 script.py {size} {core}"
            cmd = cmd + f" >> {nome}/log_{size}_{core}.txt"
            print(f"Running: '{cmd}' and saving at 'log_{size}_{core}.txt'")
            subprocess.call(cmd, shell=True)


main()