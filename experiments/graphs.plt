# vim: set ft=gnuplot

#
# Plots the requests duration
#
set title "Duração de requisições em milissegundos" font ", 24"
set datafile separator ","
set grid y
set key inside right top

set style data histogram
set style histogram cluster gap 3
set style fill solid
set boxwidth 1.5

set ylabel "Duração em (ms)" offset 1.5,0
set yrange [0:26]
set xlabel "Tamanho do corpo da resposta"

set terminal 'png' size 800,500
set output 'out/duration.png'

plot 'stats/duration-baseline.csv' using 2:xtic(1) title "baseline"



#
# Plots the Blocking duration
#
set title "Tempo de bloqueio em micro-segundos" font ", 24"

set style data histogram
set style fill solid
set boxwidth 1.5

set ylabel "Tempo (us)" offset 1.5,0
set yrange [0:12]
set xlabel "Tamanho do corpo da resposta"

set terminal 'png' size 800,600
set output 'out/block-duration.png'

plot 'stats/reqs_per_second-baseline.csv' using 2:xtic(1) linewidth 3 title "baseline"



#
# Plots requests per second
#
set title "Tempo de bloqueio em micro segundos" font ", 24"

set style data histogram
set style fill solid
set boxwidth 1

set ylabel "Requisições por segundo"
set yrange [0:25]
set xlabel "    "


plot 'stats/block_duration-parallel_cpu_userspace.csv' using 2:xtic(1) linewidth 3 title "Espaço de usuário", \
     'stats/block_duration-parallel_cpu_kernel.csv' using 2:xtic(1) linewidth 3 title "Espaço de núcleo"
