from sys import argv
from sys import exit


def get_and_write_reqs():

    reqs_userspace = 0.0
    with open("stats/reqs_per_second-userspace.csv", "r") as infile:
        data = infile.readline().split(",")
        reqs_userspace = float(data[1])

    reqs_kernel = 0.0
    with open("stats/reqs_per_second-kernel.csv", "r") as infile:
        data = infile.readline().split(",")
        reqs_kernel = float(data[1])

    with open("stats/reqs_and_cpu.csv", "a") as outfile:
        outfile.write("Vazão média, {0}, {1}\n".format(
            reqs_userspace, reqs_kernel
            )
        )
    print("[Aggregated] Type=requests")

def get_and_write_cpu_times():

    cpu_userspace = 0.0
    with open("stats/block_duration-parallel_cpu_userspace.csv", "r") as infile:
        data = infile.readline().split(",")
        cpu_userspace = float(data[1])

    cpu_kernel = 0.0
    with open("stats/block_duration-parallel_cpu_kernel.csv", "r") as infile:
        data = infile.readline().split(",")
        cpu_kernel = float(data[1])

    with open("stats/reqs_and_cpu.csv", "a") as outfile:
        outfile.write("Bloqueio, {0}, {1}\n".format(
            cpu_userspace, cpu_kernel
            )
        )
    print("[Aggregated] Type=cpu")


def aggregate_reqs_and_time():

    get_and_write_reqs()
    get_and_write_cpu_times()


def main():

    aggregate_reqs_and_time()


if __name__ == "__main__":
    main()
