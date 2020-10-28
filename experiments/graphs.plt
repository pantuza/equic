# vim: set ft=gnuplot

#
# Plots the requests duration
#
set title "Duração de requisições por tamanho de resposta" font ", 24"
set datafile separator ","
set key left
set grid

set style data histogram
set style histogram cluster gap 3
set style fill solid
set boxwidth 1.5

set ylabel "Duração em (ms)" offset 1.5,0
set yrange [0:12]
set xlabel "Tamanho do payload de resposta"

set terminal 'png' size 800,600
set output 'out/duration.png'

plot 'stats/duration-baseline.csv' using 2:xtic(1) title "baseline"



#
# Plots the requests per second curve
#
set title "Requisições por segundo por tamanho de resposta" font ", 24"

set style data lines

set ylabel "Requisições por segundo" offset 1.5,0
set yrange [0:30]
set xlabel "Tamanho do payload de resposta"

set terminal 'png' size 800,600
set output 'out/requests.png'

plot 'stats/reqs_per_second-baseline.csv' using 2:xtic(1) linewidth 3 title "baseline"
