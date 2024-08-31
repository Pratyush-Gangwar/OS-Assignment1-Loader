#invoke make inside following directories and in this order: loader, launch, fib
#move the lib_simpleloader.so and launch binaries inside bin directory
#Provide the command for cleanup

../loader/loader.out:
	cd ./loader && make && mv lib_simpleloader.so ../bin
	cd ./launcher && make && mv launch.out ../bin
	cd ./test && make && mv fib ../bin

clean:
	rm ./loader/loader.o && cd ./bin && rm * 