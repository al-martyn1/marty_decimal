@call run_test.bat benchmark_tests msvc2017 x86 Release
@copy _run_logs\benchmark_tests.log  doc\benchmark_tests.txt

@call run_test.bat benchmark_tests msvc2017 x64 Release
@echo. >> doc\benchmark_tests.txt
@echo. >> doc\benchmark_tests.txt
@echo. >> doc\benchmark_tests.txt
@echo --------------------------------------------------------------------------------------------------------------------------------------------------- >> doc\benchmark_tests.txt
@echo. >> doc\benchmark_tests.txt
@echo. >> doc\benchmark_tests.txt
@echo. >> doc\benchmark_tests.txt
@type _run_logs\benchmark_tests.log  >> doc\benchmark_tests.txt
