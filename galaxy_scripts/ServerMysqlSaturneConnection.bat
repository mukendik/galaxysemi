C:
if exist "C:/Program Files/Git/bin" cd "C:/Program Files/Git/bin"
if exist "C:/Git/bin" cd "C:/Git/bin"
bash.exe -l -c "ssh gexprod@dourdan.galaxysemi.com -p 2522 -L 3307:minas-morgul-prod:3306"
pause