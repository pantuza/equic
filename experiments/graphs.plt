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

plot 'stats/duration-userspace.csv' using 2:xtic(1) title "Espaço de usuário", \
     'stats/duration-kernel.csv' using 2:xtic(1) title "Espaço de núcleo", \
     'stats/duration-baseline.csv' using 2:xtic(1) title "Sem controle de quota"

set title "Requests duration in miliseconds" font ", 24"
set datafile separator ","
set grid y
set key inside right top

set style data histogram
set style histogram cluster gap 3
set style fill solid
set boxwidth 1.5

set ylabel "Duration (ms)" offset 1.5,0
set yrange [0:26]
set xlabel "Response body size"

set terminal 'png' size 800,500
set output 'out/duration-en.png'

plot 'stats/duration-userspace.csv' using 2:xtic(1) title "User space", \
     'stats/duration-kernel.csv' using 2:xtic(1) title "Kernel space", \
     'stats/duration-baseline.csv' using 2:xtic(1) title "No quota control"




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

plot 'stats/block_duration-parallel_cpu_userspace.csv' using 2:xtic(1) linewidth 3 title "Espaço de usuário", \
     'stats/block_duration-parallel_cpu_kernel.csv' using 2:xtic(1) linewidth 3 title "Espaço de núcleo"

set title "Blocking time in microseconds" font ", 24"

set style data histogram
set style fill solid
set boxwidth 1.5

set ylabel "Time (us)" offset 1.5,0
set yrange [0:12]
set xlabel "Response body size"

set terminal 'png' size 800,500
set output 'out/block-duration-en.png'

plot 'stats/block_duration-parallel_cpu_userspace.csv' using 2:xtic(1) linewidth 3 title "User space", \
     'stats/block_duration-parallel_cpu_kernel.csv' using 2:xtic(1) linewidth 3 title "Kernel space"


#
# Plots requests per second
#
set title "Variação na vazão de pacotes durante um ataque" font ", 24" offset screen 0,-0.03

set style data lines
set key spacing 1

set ylabel "Requisições por segundo" offset 1,0
set yrange [0:40]
set xlabel "Número da coleta (1 unidade equivale a 5 segundos)"

set terminal 'png' size 800,400
set output 'out/requests-per-second.png'

plot 'in/reqs_per_second-kernel-256k.dat' using 1 linewidth 3 title "Espaço de núcleo" smooth cspline, \
     'in/reqs_per_second-userspace-256k.dat' using 1 linewidth 3 title "Espaço de usuário" smooth cspline

set title "Throughput variation on a burst scenario" font ", 24" offset screen 0,-0.03

set style data lines
set key spacing 1

set ylabel "Requests per second" offset 1,0
set yrange [0:40]
set xlabel "Sample number (1 unit means 5 seconds)"

set terminal 'png' size 800,400
set output 'out/requests-per-second-en.png'

plot 'in/reqs_per_second-kernel-256k.dat' using 1 linewidth 3 title "Kernel space" smooth cspline, \
     'in/reqs_per_second-userspace-256k.dat' using 1 linewidth 3 title "User space" smooth cspline



#
# Plots the requests per second curve
#
clear
reset
set terminal 'png' size 800,400
set output 'out/requests.png'
set datafile separator ","

set key inside left top
set key spacing 1
set tmargin 5

set grid y
set multiplot layout 1,2


set style data histogram
set style fill solid
set boxwidth 1

set ylabel "Requisições por segundo"
set yrange [0:25]
set xlabel "    "


plot 'stats/reqs_per_second-userspace.csv' using 2:xtic("Vazão Média") title "Espaço de usuário", \
     'stats/reqs_per_second-kernel.csv' using 2:xtic("Vazão Média") title "Espaço de núcleo"


set style data histogram
set style fill solid
set boxwidth 1

unset ylabel
unset yrange
set format y "    "
set y2label "Tempo (us)" offset 1.5,0
set y2range [0:12]
set y2tics

set xlabel "Tamanho do corpo da resposta (256k)" offset screen -0.25,0
set title "Vazão média de requisições e tempo médio\n de bloqueio por contexto" font ", 24" offset screen -0.25,-0.05

plot 'stats/block_duration-parallel_cpu_userspace.csv' using 2:xtic("Bloqueio"), \
     'stats/block_duration-parallel_cpu_kernel.csv' using 2:xtic("Bloqueio")

unset multiplot

clear
reset
set terminal 'png' size 800,400
set output 'out/requests-en.png'
set datafile separator ","

set key inside left top
set key spacing 1
set tmargin 5

set grid y
set multiplot layout 1,2


set style data histogram
set style fill solid
set boxwidth 1

set ylabel "Requests per second"
set yrange [0:25]
set xlabel "    "


plot 'stats/reqs_per_second-userspace.csv' using 2:xtic("Vazão Média") title "User space", \
     'stats/reqs_per_second-kernel.csv' using 2:xtic("Vazão Média") title "Kernel space"


set style data histogram
set style fill solid
set boxwidth 1

unset ylabel
unset yrange
set format y "    "
set y2label "Time (us)" offset 1.5,0
set y2range [0:12]
set y2tics

set xlabel "Response body size (256k)" offset screen -0.25,0
set title "Averages of request throughput\nand blocking time" font ", 24" offset screen -0.25,-0.05

plot 'stats/block_duration-parallel_cpu_userspace.csv' using 2:xtic("Blocking"), \
     'stats/block_duration-parallel_cpu_kernel.csv' using 2:xtic("Blocking")

unset multiplot




#
# Plots CPU data
#
reset
set datafile separator ','

set grid
set key right bottom
set xlabel "Uso de CPU (%)"
set ylabel "Proporção (CDF)"
set yrange [0:1]

set terminal 'png' size 500,500
set output "out/cpu-cdf.png"

set title "Proporção de uso de CPU\ndurante experimento" font ", 24"

plot "stats/cpu-cdf-no-equic.csv" using 1:2 title "Sem eQUIC" linewidth 3 with linespoints, \
     "stats/cpu-cdf-equic.csv" using 1:2 title "Com eQUIC" linewidth 3 with linespoints

reset
set datafile separator ','

set grid
set key right bottom
set xlabel "CPU Usage (%)"
set ylabel "Proportion (CDF)"
set yrange [0:1]

set terminal 'png' size 500,500
set output "out/cpu-cdf-en.png"

set title "CPU usage proportion \nduring experiment" font ", 24"

plot "stats/cpu-cdf-no-equic.csv" using 1:2 title "No eQUIC Gateway" linewidth 3 with linespoints, \
     "stats/cpu-cdf-equic.csv" using 1:2 title "With eQUIC Gateway" linewidth 3 with linespoints



#
# Plots Memory data
#
reset
set datafile separator ','

set grid
set key right bottom
set xlabel "Uso de memória (%)"
set ylabel "Proporção (CDF)"
set yrange [0:1]

set terminal 'png' size 500,500
set output "out/mem-cdf.png"

set title "Proporção de uso de Memória\ndurante experimento" font ", 24"

plot "stats/mem-cdf-no-equic.csv" using 1:2 title "Sem eQUIC" linewidth 3 with linespoints, \
     "stats/mem-cdf-equic.csv" using 1:2 title "Com eQUIC" linewidth 3 with linespoints

reset
set datafile separator ','

set grid
set key right bottom
set xlabel "Memory usage (%)"
set ylabel "Proportion (CDF)"
set yrange [0:1]

set terminal 'png' size 500,500
set output "out/mem-cdf-en.png"

set title "Proportion of memory usage \nduring experiment" font ", 24"

plot "stats/mem-cdf-no-equic.csv" using 1:2 title "No eQUIC Gateway" linewidth 3 with linespoints, \
     "stats/mem-cdf-equic.csv" using 1:2 title "With eQUIC Gateway" linewidth 3 with linespoints
