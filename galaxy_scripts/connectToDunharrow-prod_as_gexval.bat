C:
if exist "C:/Program Files/Git/bin" cd "C:/Program Files/Git/bin"
if exist "C:/Program Files (x86)/Git/bin" cd "C:/Program Files (x86)/Git/bin"
if exist "C:/Git/bin" cd "C:/Git/bin"
rem bash.exe -l -c "ssh gexprod@dourdan.galaxysemi.com -p 8022 -L 80:dunharrow-prod:80"
rem bash.exe -l -c "ssh gexval@dourdan.galaxysemi.com -p 8022 -L 80:dunharrow-prod:80"
bash.exe -l -c "ssh gexval@dourdan.galaxysemi.com -p 7722 -L 80:dunharrow-prod:80"
pause
