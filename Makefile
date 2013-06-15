all:
	./waf build
test:
	echo no tests yet
install:
	python -m SimpleHTTPServer 8000 &
	echo http://`hostname`:8000/build/pebbletweet.pbw | mail pebble@ernie.org
