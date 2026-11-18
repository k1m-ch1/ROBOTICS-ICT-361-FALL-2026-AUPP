# Installation and usage

To run the program, first create a virtual environment:

```
python3 -m venv .venv
```

Activate it

```
source .venv/bin/activate
```

Install the necessary packages

```
pip install opencv-python pyserial
```

Run the program:

```
python3 ./python_code/main.py
```

Now visit main port `8080` for the main view from the camera, and port `8888` for the HSV masked view from the camera.

[Video link](https://youtu.be/G1a3E9_W4Bw?si=uM4cF6UC3GrPz9IA)
