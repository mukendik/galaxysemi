C:
if exist "C:/Program Files/Git/bin" cd "C:/Program Files/Git/bin"
if exist "C:/Git/bin" cd "C:/Git/bin"
bash.exe -l -c "ssh gexprod@dourdan.galaxysemi.com -p 4522 -L 80:khazad-dum-prod:80"
pause
