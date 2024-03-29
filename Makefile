CFLAGS = $(shell pkg-config --cflags glfw3 glew glm libjpeg)
LDFLAGS = $(shell pkg-config --libs glfw3 glew glm libjpeg)
hw3:
	g++ Main.cpp EclipseMap.cpp Shader.cpp -o hw3 -std=c++11 -lXi -lGLEW -lGLU -lm -lGL -lm -lpthread -ldl -ldrm -lXdamage  -lglfw3 -lrt -lm -ldl -lXrandr -lXinerama -lXxf86vm -lXext -lXcursor -lXrender -lXfixes -lX11 -lpthread -ljpeg
local:
	g++ Main.cpp EclipseMap.cpp Shader.cpp -o hw3 -std=c++11 $(CFLAGS) $(LDFLAGS)
clean:
	rm hw3
