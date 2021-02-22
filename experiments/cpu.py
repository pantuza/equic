from statistics import NormalDist
from statistics import mean
from statistics import stdev



def extract(logfile, output):
    """ Extracts CPU data from log file and outputs it to input directory """

    ofile = open(output, "w")

    with open(logfile) as infile:

        # Skip first two lines
        infile.readline()
        infile.readline()
        infile.readline()

        for line in infile:

            data = line.split()

            ofile.write("{0}\n".format(data[7]))

            # skips title and blank line
            infile.readline()
            infile.readline()

    ofile.close()


def compute(infile, outfile):

    ofile = open(outfile, "w")

    cpu = []
    with open(infile) as indata:

        for line in indata:
            cpu.append(float(line))

    mu = mean(cpu)
    sigma = stdev(cpu)

    dist = NormalDist(mu=mu, sigma=sigma)

    for x in sorted(cpu):
        ofile.write("{0}, {1}\n".format(x, dist.cdf(x)))

    ofile.close()

def main():

    extract("../logs/pid-stats-no-equic-parallel.log", "in/cpu-no-equic.dat")
    extract("../logs/pid-stats-equic-parallel.log", "in/cpu-equic.dat")

    compute("in/cpu-no-equic.dat", "stats/cpu-cdf-no-equic.csv")
    compute("in/cpu-equic.dat", "stats/cpu-cdf-equic.csv")


if __name__ == "__main__":
    main()
