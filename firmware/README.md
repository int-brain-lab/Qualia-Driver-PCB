Read an existing program binary into a file named `avr.hex`:

````
avrdude -v -V -p attiny85 -c usbtiny -D -Uflash:r:avr.hex:i
````

Program an existing attiny85 with the file named `avr.hex`
````
avrdude -v -V -p attiny85 -c usbtiny -D -Uflash:w:avr.hex:i
````
