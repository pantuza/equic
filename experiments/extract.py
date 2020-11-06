from sys import argv
from sys import exit


# The output here is the input for the statistics computation
OUT_DIR = "in/"


def extract_request_time(data, base_name):

    splitted = data.split("=")

    key = splitted[0]

    if key != "RequestDuration":
        return

    value = splitted[1][:-1]

    ofilename = OUT_DIR + "duration-" + base_name + ".dat"

    with open(ofilename, "a") as ofile:
        ofile.write("{0}\n".format(value))


def extract_requests_per_second(data, base_name):

    splitted = data.split("=")

    key = splitted[0]

    if key != "RequestsPerSecond":
        return

    value = splitted[1]

    if int(value) > 0:
        ofilename = OUT_DIR + "reqs_per_second-" + base_name + ".dat"

        with open(ofilename, "a") as ofile:
            ofile.write("{0}\n".format(value))


def extract_cpu_time(data, base_name):

    splitted = data.split("=")

    key = splitted[0]

    if key != "BlockDuration":
        return

    value = splitted[1][:-1]

    if float(value) > 0.0:
        ofilename = OUT_DIR + "block_duration-" + base_name + ".dat"

        with open(ofilename, "a") as ofile:
            ofile.write("{0}\n".format(float(value)))


def extract(log):

    test_type = log.split("-")[-2]  # for example: baseline
    test_size = log.split("-")[-1][:-4]  # for example: 32k

    # Output file base name
    base_name = test_type + "-" + test_size

    with open(log) as in_file:
        for line in in_file:

            data = line.split()
            extract_request_time(data[1], base_name)
            extract_requests_per_second(data[1], base_name)
            extract_cpu_time(data[1], base_name)  # Extracts for Userspace
            if len(data) > 6:
                extract_cpu_time(data[6], base_name)  # Extracts for Kernelspace

    print("[Extracted] Type={0}, Size={1}, To={2}".format(
        test_type, test_size, OUT_DIR)
    )


def main():
    if not len(argv) > 1:
        print("[eQUIC Analysis] Missing log file as input")
        exit(1)

    log = argv[1]

    extract(log)


if __name__ == "__main__":
    main()
