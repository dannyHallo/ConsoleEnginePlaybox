@REM To use this script, simply run it in a Git repository directory. 
@REM The script will generate a random 4-character hex string and use 
@REM it as a commit message to commit and push changes to the remote repository.

@REM PLEASE NOTICE: This script does not handle the logic for pulling from remote repository.

@echo off
setlocal enabledelayedexpansion
set "string=0123456789ABCDEF"

set /a x=%random% %% 16 
set no1=%result%!string:~%x%,1!
set /a x=%random% %% 16 
set no2=%result%!string:~%x%,1!
set /a x=%random% %% 16 
set no3=%result%!string:~%x%,1!
set /a x=%random% %% 16 
set no4=%result%!string:~%x%,1!

set result=CKPT_%no1%%no2%%no3%%no4%
echo %result%

git status
git add .
git commit -m "%result%"
git push
git status

echo "Successfully pushed to remote repository, with commit message: %result%"