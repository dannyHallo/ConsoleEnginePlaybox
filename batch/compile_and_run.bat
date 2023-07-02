set PROJECT_NAME=CCPlayground
set BUILD_TYPE=Release

@echo *********************** cmake ***********************
mkdir build
cd build
cmake .. -D PROJ_NAME=%PROJECT_NAME%
cd ..

@echo *********************** msbuild ***********************
MSBuild ./build/%PROJECT_NAME%.sln -p:Configuration=%BUILD_TYPE%

@echo *********************** run ***********************
start /d "./build/%BUILD_TYPE%" %PROJECT_NAME%.exe