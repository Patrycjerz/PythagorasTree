#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <typeinfo>
#include <SFML/Window.hpp>
#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/rotate_vector.hpp>

bool createShader(const std::string& file_name, const GLenum type, GLuint& shader)
{
	GLuint new_shader = glCreateShader(type);

	if (new_shader == 0)
	{
		std::cerr << "Cannot create " + std::to_string(new_shader) + " shader" << std::endl;
		glGetError();
		std::cin.get();
		return false;
	}
	else
	{
		std::ifstream file(file_name);
		if (!file.good())
		{
			std::cerr << "Cannot open " + file_name + " shader file" << std::endl;
			std::cin.get();
			return false;
		}

		std::string content;
		std::string line;
		while (std::getline(file, line))
		{
			content += line;
			content += "\x0a";
		}

		file.close();

		const GLchar* const content_ptr = content.c_str();
		glShaderSource(new_shader, 1, &content_ptr, NULL);

		glCompileShader(new_shader);
		GLint status;
		glGetShaderiv(new_shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLint size;
			glGetShaderiv(new_shader, GL_INFO_LOG_LENGTH, &size);
			GLchar* const log = new GLchar[size];
			glGetShaderInfoLog(new_shader, size, NULL, log);
			std::cerr << "Cannot compile " + std::to_string(new_shader) + " shader" << std::endl << log << std::endl;
			delete[] log;
			std::cin.get();
			return false;
		}
	}

	shader = new_shader;

	return true;
}

bool createProgram(std::vector<GLuint> shaders, GLuint& program)
{
	GLuint new_program = glCreateProgram();

	if (new_program == 0)
	{
		std::cerr << "Cannot create " + std::to_string(new_program) + " program" << std::endl;
		glGetError();
		std::cin.get();
		return false;
	}
	else
	{
		for (const GLuint i : shaders)
		{
			glAttachShader(new_program, i);

			GLenum error;
			if ((error = glGetError()) != GL_NO_ERROR)
			{
				std::cerr << "Attach shader error: " << error << std::endl;
				std::cin.get();
				return false;
			}
		}

		glLinkProgram(new_program);
		GLint status;
		glGetProgramiv(new_program, GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			std::cerr << "Cannot link " + std::to_string(new_program) + " program" << std::endl;
			std::cin.get();
			return false;
		}

		glValidateProgram(new_program);
		glGetProgramiv(new_program, GL_VALIDATE_STATUS, &status);
		if (status == GL_FALSE)
		{
			std::cerr << "Cannot validate " + std::to_string(new_program) + " program" << std::endl;
			std::cin.get();
			return false;
		}
	}

	program = new_program;

	return true;
}

template<class T>
bool loadSetting(std::fstream& file, int labels_numbers_of_words[], T& setting, int line_number)
{
	std::string line;
	for (int i = 0; i < line_number; i++)
	{
		std::getline(file, line);
	}

	if (!file)
	{
		std::cerr << "Bad data in settings file" << std::endl;
		std::cin.get();
		return false;
	}

	std::stringstream sstream;
	sstream << line;
	std::string s;
	for (int i = 0; i < labels_numbers_of_words[line_number - 1]; i++)
		sstream >> s;

	if (typeid(setting) == typeid(bool))
		sstream >> std::boolalpha >> setting;
	else if (typeid(setting) == typeid(int))
		sstream >> std::hex >> setting;
	else
		sstream >> setting;

	if (!sstream)
	{
		std::cerr << "Bad data in settings file" << std::endl;
		std::cin.get();
		return false;
	}

	file.seekg(0);

	return true;
}

bool loadSettings(const std::string& file_name,
                  bool&              is_3d,
                  int&               iters,
                  GLfloat&           side,
                  GLfloat&           depth,
                  float&             angle,
                  glm::vec3&         first_color,
                  glm::vec3&         last_color,
                  bool&              reversing,
                  bool&              is_directed_light,
                  bool&              is_dynamic_light)
{
	std::fstream file(file_name);
	if (!file.good())
	{
		std::cerr << "Cannot open " + file_name + " settings file" << std::endl;
		std::cin.get();
		return false;
	}

	int labels_numbers_of_words[10] = {2, 1, 2, 2, 2, 3, 3, 3, 3, 3};

	if (!loadSetting(file, labels_numbers_of_words, is_3d, 1) ||
	    !loadSetting(file, labels_numbers_of_words, iters, 2) ||
	    !loadSetting(file, labels_numbers_of_words, side, 3)  ||
	    !loadSetting(file, labels_numbers_of_words, depth, 4) ||
	    !loadSetting(file, labels_numbers_of_words, angle, 5))
	{
		return false;
	}

	int first_color_int;
	if (!loadSetting(file, labels_numbers_of_words, first_color_int, 6))
		return false;

	first_color.x = (first_color_int & 0xff0000) / glm::pow(255.f, 3);
	first_color.y = (first_color_int & 0xff00) / glm::pow(255.f, 2);
	first_color.z = (first_color_int & 0xff) / 255.f;

	int last_color_int;
	if (!loadSetting(file, labels_numbers_of_words, last_color_int, 7))
		return false;

	last_color.x = (last_color_int & 0xff0000) / glm::pow(255.f, 3);
	last_color.y = (last_color_int & 0xff00) / glm::pow(255.f, 2);
	last_color.z = (last_color_int & 0xff) / 255.f;

	if (!loadSetting(file, labels_numbers_of_words, reversing, 8)         ||
	    !loadSetting(file, labels_numbers_of_words, is_directed_light, 9) ||
	    !loadSetting(file, labels_numbers_of_words, is_dynamic_light, 10))
	{
		return false;
	}

	if (!file)
	{
		std::cerr << "Bad data in settings file" << std::endl;
		std::cin.get();
		return false;
	}

	return true;
}

void _genPythagorasTree(const GLfloat         side,
                        const GLfloat         depth,
                        const float           angle,
                        const float           abs_angle,
                        const int             iter,
                        const int             last_iter,
                        const float           prev_side,
                        const bool            is_left,
                        const bool            prev_is_left,
                        const bool            reversing,
                        glm::mat4             trans,
                        std::vector<GLfloat>& vertices,
                        std::vector<GLfloat>& normals,
                        std::vector<int>&     iters)
{
	if (iter == last_iter + 1)
		return;

	glm::vec3 offset(prev_side, 0.f, 0.f);
	if (is_left)
		offset = glm::rotate(offset, glm::radians(abs_angle - angle), glm::vec3(0.f, 0.f, 1.f));
	else
		offset = glm::rotate(offset, glm::radians(abs_angle + 90.f - angle), glm::vec3(0.f, 0.f, 1.f));
	offset = glm::rotate(offset, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
	trans = glm::translate(trans, offset);

	if (prev_is_left)
	{
		if (!is_left)
		{
			offset = glm::rotate(offset, glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f));
			trans = glm::translate(trans, offset);
		}
	}
	else
	{
		if (is_left)
		{
			offset = glm::rotate(offset, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
			trans = glm::translate(trans, offset);
		}
	}

	glm::vec4 v[4] =
	{
		glm::vec4(0.f, side, depth, 1.f),
		glm::vec4(side, side, depth, 1.f),
		glm::vec4(side, 0.f, depth, 1.f),
		glm::vec4(0.f, 0.f, depth, 1.f)
	};

	glm::vec2 n[4] =
	{
		glm::vec2(-1.f,  1.f),
		glm::vec2(1.f,  1.f),
		glm::vec2(1.f, -1.f),
		glm::vec2(-1.f, -1.f)
	};

	if (is_left)
	{
		for (int i = 0; i < 4; i++)
		{
			v[i] = glm::rotate(v[i], glm::radians(abs_angle), glm::vec3(0.f, 0.f, 1.f));
			n[i] = glm::rotate(n[i], glm::radians(abs_angle));
		}
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			v[i] = glm::rotate(v[i], glm::radians(abs_angle + 90.f), glm::vec3(0.f, 0.f, 1.f));
			n[i] = glm::rotate(n[i], glm::radians(abs_angle + 90.f));
		}
	}

	for (int i = 0; i < 4; i++)
		v[i] = trans * v[i];

	for (int i = 0; i < 4; i++)
	{
		vertices.push_back(v[i].x);
		vertices.push_back(v[i].y);
		vertices.push_back(v[i].z / 2.f);

		normals.push_back(n[i].x);
		normals.push_back(n[i].y);
		normals.push_back(1.f);
	}
	for (int i = 0; i < 4; i++)
	{
		vertices.push_back(v[i].x);
		vertices.push_back(v[i].y);
		vertices.push_back(-v[i].z / 2.f);

		normals.push_back(n[i].x);
		normals.push_back(n[i].y);
		normals.push_back(-1.f);
	}

	iters.push_back(iter);

	if (iter == 1 || !reversing)
	{
		_genPythagorasTree(glm::sin(glm::radians(180.f - 90.f - angle)) * side,
		                   depth,
		                   angle,
		                   abs_angle + angle,
		                   iter + 1,
		                   last_iter,
		                   side,
		                   true,
		                   is_left,
		                   reversing,
		                   trans,
		                   vertices,
		                   normals,
		                   iters);

		_genPythagorasTree(glm::sin(glm::radians(angle)) * side,
		                   depth,
		                   angle,
		                   abs_angle - (90.f - angle),
		                   iter + 1,
		                   last_iter,
		                   side,
		                   false,
		                   is_left,
		                   reversing,
		                   trans,
		                   vertices,
		                   normals,
		                   iters);
	}
	else if (iter % 2 == 1 && reversing)
	{
		_genPythagorasTree(glm::sin(glm::radians(angle)) * side,
		                   depth,
		                   90.f - angle,
		                   abs_angle + (90.f - angle),
		                   iter + 1,
		                   last_iter,
		                   side,
		                   true,
		                   is_left,
		                   reversing,
		                   trans,
		                   vertices,
		                   normals,
		                   iters);

		_genPythagorasTree(glm::sin(glm::radians(180.f - 90.f - angle)) * side,
		                   depth,
		                   90.f - angle,
		                   abs_angle - angle,
		                   iter + 1,
		                   last_iter,
		                   side,
		                   false,
		                   is_left,
		                   reversing, 
		                   trans,
		                   vertices,
		                   normals,
		                   iters);
	}
	else if (reversing)
	{
		_genPythagorasTree(glm::sin(glm::radians(angle)) * side,
		                   depth,
		                   90.f - angle,
		                   abs_angle + (90.f - angle),
		                   iter + 1,
		                   last_iter,
		                   side,
		                   true,
		                   is_left,
		                   reversing,
		                   trans,
		                   vertices,
		                   normals,
		                   iters);

		_genPythagorasTree(glm::sin(glm::radians(180.f - 90.f - angle)) * side,
		                   depth,
		                   90.f - angle,
		                   abs_angle - angle,
		                   iter + 1,
		                   last_iter,
		                   side,
		                   false,
		                   is_left,
		                   reversing,
		                   trans,
		                   vertices,
		                   normals,
		                   iters);
	}
}

bool genPythagorasTree(const GLfloat         side,
                       const GLfloat         depth,
                       const float           angle,
                       const int             iters,
                       const bool            reversing,
                       std::vector<GLfloat>& vertices,
                       std::vector<GLfloat>& normals,
                       std::vector<int>&     _iters)
{
	if (side <= 0.f || angle <= 0.f || angle >= 90.f || iters <= 0)
	{
		std::cerr << "Wrong Pythagoras Tree function argument(s)" << std::endl;
		std::cin.get();
		return false;
	}

	_genPythagorasTree(side, depth, angle, 0.f, 1, iters, 0.f, true, true, reversing, glm::mat4(), vertices, normals, _iters);

	return true;
}

int main()
{
	const int window_x = 600;
	const int window_y = 600;

	sf::Window window(sf::VideoMode(window_x, window_y), "PythagorasTree", sf::Style::Default, sf::ContextSettings(24, 0, 8, 4, 3));
	window.setFramerateLimit(100);

	if (glewInit() != GLEW_OK)
	{
		std::cerr << "GLEW init error" << std::endl;
		std::cin.get();
		return EXIT_FAILURE;
	}

	if (!GLEW_VERSION_3_2)
	{
		std::cerr << "Too old OpenGL version" << std::endl;
		std::cin.get();
		return EXIT_FAILURE;
	}

	std::vector<GLfloat> vertices;
	std::vector<GLfloat> normals;
	std::vector<int> _iters;

	bool      is_3d;
	int       iters;
	GLfloat   side;
	GLfloat   depth;
	float     angle;
	glm::vec3 first_color;
	glm::vec3 last_color;
	bool      reversing;
	bool      is_directed_light;
	bool      is_dynamic_light;

	if (!loadSettings("settings.txt", is_3d, iters, side, depth, angle, first_color, last_color, reversing, is_directed_light, is_dynamic_light))
		return EXIT_FAILURE;

	if (!genPythagorasTree(side, depth, angle, iters, reversing, vertices, normals, _iters))
		return EXIT_FAILURE;

	GLuint vert_shader;
	if (!createShader("pt_vertex_shader.vert", GL_VERTEX_SHADER, vert_shader))
		return EXIT_FAILURE;

	GLuint frag_shader;
	if (!createShader("pt_fragment_shader.frag", GL_FRAGMENT_SHADER, frag_shader))
		return EXIT_FAILURE;

	std::vector<GLuint> shaders;
	shaders.push_back(vert_shader);
	shaders.push_back(frag_shader);

	GLuint program;
	if (!createProgram(shaders, program))
		return EXIT_FAILURE;

	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	GLuint vertices_buffer;
	glGenBuffers(1, &vertices_buffer);
		
	glBindBuffer(GL_ARRAY_BUFFER, vertices_buffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

	GLenum error;
	if ((error = glGetError()) != GL_NO_ERROR)
	{
		std::cerr << "Buffer data error: " << error << std::endl;
		std::cin.get();
		return EXIT_FAILURE;
	}

	glVertexAttribPointer(glGetAttribLocation(program, "in_position"), 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(glGetAttribLocation(program, "in_position"));

	GLuint normals_buffer;
	glGenBuffers(1, &normals_buffer);

	glBindBuffer(GL_ARRAY_BUFFER, normals_buffer);

	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(glGetAttribLocation(program, "in_normal"), 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(glGetAttribLocation(program, "in_normal"));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	GLuint elements_buffer;
	glGenBuffers(1, &elements_buffer);

	std::vector<GLuint> elements;

	elements.push_back(0);
	elements.push_back(1);
	elements.push_back(2);

	elements.push_back(0);
	elements.push_back(2);
	elements.push_back(3);

	elements.push_back(1);
	elements.push_back(5);
	elements.push_back(6);

	elements.push_back(1);
	elements.push_back(6);
	elements.push_back(2);

	elements.push_back(5);
	elements.push_back(4);
	elements.push_back(7);

	elements.push_back(5);
	elements.push_back(7);
	elements.push_back(6);

	elements.push_back(4);
	elements.push_back(0);
	elements.push_back(3);

	elements.push_back(4);
	elements.push_back(3);
	elements.push_back(7);

	elements.push_back(4);
	elements.push_back(5);
	elements.push_back(1);

	elements.push_back(4);
	elements.push_back(1);
	elements.push_back(0);

	elements.push_back(3);
	elements.push_back(2);
	elements.push_back(6);

	elements.push_back(3);
	elements.push_back(6);
	elements.push_back(7);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(GLuint), elements.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glClearColor(1.f, 1.f, 1.f, 1.f);

	float window_ratio = static_cast<float>(window_x) / window_y;

	glm::vec3 light_direction(0.f, 0.f, -1.f);

	glm::vec3 camera_position(0.f, 0.f, 7.f);
	float camera_angle_x = 0.f;
	glm::vec3 camera_axis_x(-1.f, 0.f, 0.f);
	const float camera_velocity = 0.175f;

	glm::vec2 ortho_position(0.f, 0.f);
	const float ortho_velocity = 0.005f;
	const glm::vec2 ortho_area(20.f, 20.f);

	float ortho_x = 4.f;
	float ortho_y = ortho_x / window_ratio;

	sf::Vector2i old_mouse_pos = sf::Mouse::getPosition(window);

	sf::Event event;

	bool is_closed = false;

	while (true)
	{
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
				is_closed = true;
			}

			if (event.type == sf::Event::Resized)
			{
				glViewport(0, 0, window.getSize().x, window.getSize().y);
				window_ratio = static_cast<float>(window.getSize().x) / window.getSize().y;
				ortho_y = ortho_x / window_ratio;
			}

			if (event.type == sf::Event::MouseWheelScrolled)
			{
				if (is_3d)
				{
					glm::vec3 camera_position_normalized(camera_position.x / glm::length(camera_position),
														 camera_position.y / glm::length(camera_position),
														 camera_position.z / glm::length(camera_position));

					const float camera_zoom_velocity = 0.2f;
					glm::vec3 camera_zoom_delta(camera_position_normalized.x * camera_zoom_velocity * -event.mouseWheelScroll.delta,
												camera_position_normalized.y * camera_zoom_velocity * -event.mouseWheelScroll.delta,
												camera_position_normalized.z * camera_zoom_velocity * -event.mouseWheelScroll.delta);

					if (glm::length(camera_position + camera_zoom_delta) > 2.f &&
						glm::length(camera_position + camera_zoom_delta) < 30.f)
					{
						camera_position += camera_zoom_delta;
					}
				}
				else
				{
					const float ortho_zoom_velocity = 0.2f;

					if (ortho_x + ortho_zoom_velocity * -event.mouseWheelScroll.delta > 1.f &&
					    ortho_x + ortho_zoom_velocity * -event.mouseWheelScroll.delta < 15.f)
					{
						ortho_x += ortho_zoom_velocity * -event.mouseWheelScroll.delta;
						ortho_y = ortho_x / window_ratio;
					}
				}
			}
		}

		if (is_closed)
			break;

		sf::Vector2i current_mouse_pos = sf::Mouse::getPosition(window);

		sf::Vector2i mouse_delta;
		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
			mouse_delta = current_mouse_pos - old_mouse_pos;

		old_mouse_pos = current_mouse_pos;

		if (mouse_delta.y > 0)
		{
			if (is_3d)
			{
				float camera_angle_delta_x;

				camera_angle_x += camera_velocity * mouse_delta.y;
				if (camera_angle_x > 89.f)
				{
					camera_angle_x -= camera_velocity * mouse_delta.y;
					camera_angle_delta_x = 89.f - camera_angle_x;
					camera_angle_x = 89.f;
				}
				else
					camera_angle_delta_x = camera_velocity * mouse_delta.y;

				camera_position = glm::rotate(camera_position, glm::radians(camera_angle_delta_x), camera_axis_x);
			}
			else
			{
				if (ortho_position.y + ortho_velocity * mouse_delta.y > ortho_area.y / 2 - ortho_y / 2)
					ortho_position.y = ortho_area.y / 2 - ortho_y / 2;
				else
					ortho_position.y += ortho_velocity * mouse_delta.y;
			}
		}
		else if (mouse_delta.y < 0)
		{
			if (is_3d)
			{
				float camera_angle_delta_x;

				camera_angle_x -= camera_velocity * -mouse_delta.y;
				if (camera_angle_x < -89.f)
				{
					camera_angle_x += camera_velocity * -mouse_delta.y;
					camera_angle_delta_x = -89.f - camera_angle_x;
					camera_angle_x = -89.f;
				}
				else
					camera_angle_delta_x = -camera_velocity * -mouse_delta.y;

				camera_position = glm::rotate(camera_position, glm::radians(camera_angle_delta_x), camera_axis_x);
			}
			else
			{
				if (ortho_position.y - ortho_velocity * -mouse_delta.y < -ortho_area.y / 2 + ortho_y / 2)
					ortho_position.y = -ortho_area.y / 2 + ortho_y / 2;
				else
					ortho_position.y -= ortho_velocity * -mouse_delta.y;
			}
		}
		if (mouse_delta.x < 0)
		{
			if (is_3d)
			{
				camera_position = glm::rotate(camera_position, glm::radians(camera_velocity * -mouse_delta.x), glm::vec3(0.f, 1.f, 0.f));
				camera_axis_x = glm::rotate(camera_axis_x, glm::radians(camera_velocity * -mouse_delta.x), glm::vec3(0.f, 1.f, 0.f));
			}
			else
			{
				if (ortho_position.x + ortho_velocity * -mouse_delta.x > ortho_area.x / 2 - ortho_x / 2)
					ortho_position.x = ortho_area.x / 2 - ortho_x / 2;
				else
					ortho_position.x += ortho_velocity * -mouse_delta.x;
			}
		}
		else if (mouse_delta.x > 0)
		{
			if (is_3d)
			{
				camera_position = glm::rotate(camera_position, glm::radians(-camera_velocity * mouse_delta.x), glm::vec3(0.f, 1.f, 0.f));
				camera_axis_x = glm::rotate(camera_axis_x, glm::radians(-camera_velocity * mouse_delta.x), glm::vec3(0.f, 1.f, 0.f));
			}
			else
			{
				if (ortho_position.x - ortho_velocity * mouse_delta.x < -ortho_area.x / 2 + ortho_x / 2)
					ortho_position.x = -ortho_area.x / 2 + ortho_x / 2;
				else
					ortho_position.x -= ortho_velocity * mouse_delta.x;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program);

		glm::mat4 view_matrix;
		if (is_3d)
			view_matrix = glm::lookAt(camera_position, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
		else
			view_matrix = glm::translate(glm::vec3(-ortho_position.x, -ortho_position.y, -7.f));

		glm::mat4 projection_matrix;
		if (is_3d)
			projection_matrix = glm::perspective(glm::radians(45.f), window_ratio, 1.f, 50.f);
		else
			projection_matrix = glm::ortho(-ortho_x / 2, ortho_x / 2, -ortho_y / 2, ortho_y / 2, 1.f, 50.f);

		glm::mat4 model_matrix;
		model_matrix = glm::translate(model_matrix, glm::vec3(-side / 2, -0.5f, 0.f));

		glm::mat4 mvp;
		mvp = projection_matrix * view_matrix * model_matrix;

		if (is_dynamic_light)
			light_direction = glm::rotate(light_direction, glm::radians(0.3f), glm::vec3(0.f, 1.f, 0.f));

		glUniformMatrix4fv(glGetUniformLocation(program, "mvp_matrix"), 1, GL_FALSE, glm::value_ptr(mvp));
		glUniform3f(glGetUniformLocation(program, "first_color"), first_color.x, first_color.y, first_color.z);
		glUniform3f(glGetUniformLocation(program, "last_color"), last_color.x, last_color.y, last_color.z);
		glUniform1i(glGetUniformLocation(program, "is_directed_light"), is_directed_light);
		glUniform3f(glGetUniformLocation(program, "light_direction"), light_direction.x, light_direction.y, light_direction.z);

		glBindVertexArray(vertex_array);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements_buffer);

		for (int i = 0; i < vertices.size() / 24; i++)
		{
			glUniform1f(glGetUniformLocation(program, "ratio"), 1.f - (_iters[i] - 1) * (1.f / (iters - 1)));

			glDrawElementsBaseVertex(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, NULL, i * 8);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		window.display();
	}

	return EXIT_SUCCESS;
}