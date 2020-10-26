from sys import argv
from sys import exit

from statistics import mean


OUT_DIR = "stats/"


def compute_avg_duration(dat):

    durations = []

    with open(dat) as ifile:

        for duration in ifile:
            durations.append(float(duration))

    return mean(durations)

def compute(dat, meta):

    ofilename = OUT_DIR + meta["metric"] + "-" + meta["type"] + ".csv"

    avg_duration = compute_avg_duration(dat)

    with open(ofilename, "a") as ofile:
        ofile.write("{0}, {1}\n".format(meta["size"], avg_duration))

    print("[Computed] Type={0}, Size={1}, To={2}".format(
        meta["type"], meta["size"], OUT_DIR)
    )


def main():

    if not len(argv) > 1:
        print("[eQUIC Analysis] Missing log file as input")
        exit(1)

    dat = argv[1]

    splitted = dat.split("-")

    meta = {
        "metric": splitted[0].split("/")[-1],       # For example: duration
        "type": splitted[1],                        # For example: baseline
        "size": splitted[2][:-4],                   # For example: 32k
    }

    compute(dat, meta)


if __name__ == "__main__":
    main()
