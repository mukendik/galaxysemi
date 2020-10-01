rem call MV_UVGROUPS -t tests-20 -r tests-20-uv
rem call MV_RUN 261086-P3N -d 261086 -r tests-20-uv
rem call MV_GRAPH "261086/261086-1-P3N" -o "261086/261086-1-P3N_tests-20-uv" -g 18

rem call MV_RUN 261086-P3N -d 261086 -r tests-20-log-uv
rem call MV_GRAPH "261086/261086-1-P3N" -o "261086/261086-1-P3N_tests-20-log-uv" -g 18

rem call MV_RUN 261086-P3N -d 261086 -r tests-20-sqrt-uv

rem call MV_UVGROUPS -t tests-pat-log -r tests-pat-log-uv
rem call MV_RUN 261086-P3N -d 261086 -r tests-pat-log-uv
rem call MV_GRAPH "261086/261086-1-P3N" -o "261086/261086-1-P3N_tests-pat-log-uv" -g 8

rem call MV_SINGLEGROUP -t tests-20-log -r tests-20-log
rem call MV_RUN 261086-P3N -d 261086 -r tests-20-log -c tests-20-uv-log
rem call MV_GRAPH "261086/261086-1-P3N" -o "261086/261086-1-P3N_tests-20-log" -m 20 -p -f -s
rem call MV_GRAPH "261086/261086-1-P3N" -o "261086/261086-1-P3N_tests-20-log" -m 0 -p > PC_details.txt
rem call MV_GRAPH "261086/261086-1-P3N" -o "261086/261086-1-P3N_tests-20-log" -m 20 -f -s

rem call MV_RUN 261086-P3N -d 261086 -r tests-17-uv
rem call MV_GRAPH "261086/261086-1-P3N" -o "261086/261086-1-P3N_tests-17-uv" -g 7
rem call MV_GRAPH "261086/261086-2-P3N" -o "261086/261086-2-P3N_tests-17-uv" -g 7

rem call MV_RUN 261086-P3N -d 261086 -r tests-17 -c tests-17-uv
rem call MV_GRAPH "261086/261086-1-P3N" -o "261086/261086-1-P3N_tests-17" -m 20 -p -f -s
rem call MV_GRAPH "261086/261086-2-P3N" -o "261086/261086-2-P3N_tests-17" -m 20 -p -f -s

rem call MV_RUN 261249-P3N -d 261249 -r tests-17-uv
rem call MV_RUN 261249-P3N -d 261249 -r tests-17 -c tests-17-uv

rem call MV_RUN 261320-P3N -d 261320 -r tests-17-uv
rem call MV_RUN 261320-P3N -d 261320 -r tests-17 -c tests-17-uv

rem call MV_RUN 261465-P3N -d 261465 -r tests-17-uv
rem call MV_RUN 261465-P3N -d 261465 -r tests-17 -c tests-17-uv

rem call MV_RUN 261249-P3N -d 261249 -r tests-20-log-uv
rem call MV_RUN 261249-P3N -d 261249 -r tests-20-log -c tests-20-log-uv

rem call MV_RUN 261320-P3N -d 261320 -r tests-20-log-uv
rem call MV_RUN 261320-P3N -d 261320 -r tests-20-log -c tests-20-log-uv

rem call MV_RUN 261465-P3N -d 261465 -r tests-20-log-uv
rem call MV_RUN 261465-P3N -d 261465 -r tests-20-log -c tests-20-log-uv

rem call MV_RUN 261086-P3N -d 261086 -r tests-pat-log-pca20 -c tests-pat-log-uv
rem call MV_RUN 261086-P3N -d 261086 -r tests-pat-log -c tests-pat-log-uv
rem call MV_RUN 261086-P3N -d 261086 -r tests-pat-groups -c tests-pat-log-uv

rem call MV_RUN 261249-P3N -d 261249 -r tests-pat-log-uv
rem call MV_RUN 261320-P3N -d 261320 -r tests-pat-log-uv
rem call MV_RUN 261465-P3N -d 261465 -r tests-pat-log-uv

rem call MV_RUN 261249-P3N -d 261249 -r tests-pat-log-pca20 -c tests-pat-log-uv
rem call MV_RUN 261320-P3N -d 261320 -r tests-pat-log-pca20 -c tests-pat-log-uv
rem call MV_RUN 261465-P3N -d 261465 -r tests-pat-log-pca20 -c tests-pat-log-uv

rem call MV_RUN 261249-P3N -d 261249 -r tests-pat-log -c tests-pat-log-uv
rem call MV_RUN 261320-P3N -d 261320 -r tests-pat-log -c tests-pat-log-uv
rem call MV_RUN 261465-P3N -d 261465 -r tests-pat-log -c tests-pat-log-uv

rem call MV_RUN 261249-P3N -d 261249 -r tests-pat-groups -c tests-pat-log-uv
rem call MV_RUN 261320-P3N -d 261320 -r tests-pat-groups -c tests-pat-log-uv
rem call MV_RUN 261465-P3N -d 261465 -r tests-pat-groups -c tests-pat-log-uv
