# vim: set ft=gnuplot

set title "Duração de requisições por tamanho de resposta" font ", 24"
set datafile separator ","
set key left
set grid

set style data histogram
set style histogram cluster gap 3
set style fill solid
set boxwidth 1.5

set ylabel "Duração em (ms)" offset 1.5,0
#set yrange [0:100]
set xlabel "Tamanho do payload de resposta"

set terminal 'png' size 800,600
set output 'out/duration.png'

plot 'stats/duration-baseline.csv' using 2:xtic(1) title "baseline"
