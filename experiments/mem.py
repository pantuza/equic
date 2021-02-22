from statistics import NormalDist
from statistics import mean
from statistics import stdev



def extract(logfile, output):
    """ Extracts Memory data from log file and outputs it to input directory """

    ofile = open(output, "w")

    with open(logfile) as infile:

        # Skip first two lines
        infile.readline()
        infile.readline()
        infile.readline()

        for line in infile:

            data = line.split()

            ofile.write("{0}\n".format(data[13]))

            # skips title and blank line
            infile.readline()
            infile.readline()

    ofile.close()


def compute(infile, outfile):

    ofile = open(outfile, "w")

    mem = []
    with open(infile) as indata:

        for line in indata:
            mem.append(float(line))

    mu = mean(mem)
    sigma = stdev(mem)

    dist = NormalDist(mu=mu, sigma=sigma)

    for x in sorted(mem):
        ofile.write("{0}, {1}\n".format(x, dist.cdf(x)))

    ofile.close()

def main():

    extract("../logs/pid-stats-no-equic-parallel.log", "in/mem-no-equic.dat")
    extract("../logs/pid-stats-equic-parallel.log", "in/mem-equic.dat")

    compute("in/cpu-no-equic.dat", "stats/mem-cdf-no-equic.csv")
    compute("in/cpu-equic.dat", "stats/mem-cdf-equic.csv")


if __name__ == "__main__":
    main()
