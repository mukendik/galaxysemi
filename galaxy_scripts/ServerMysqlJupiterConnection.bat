C:
if exist "C:/Program Files/Git/bin" cd "C:/Program Files/Git/bin"
if exist "C:/Git/bin" cd "C:/Git/bin"
bash.exe -l -c "ssh gexprod@dourdan.galaxysemi.com -p 7722 -L 3307:dunharrow-prod:3306"
pause