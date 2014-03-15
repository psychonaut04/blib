#pragma once


#include <map>
#include <string>
#include <glm/glm.hpp>

namespace blib
{
	class Shader
	{
	public:
		std::map<std::string, int> attributes;
		std::string vertexShader;
		std::string fragmentShader;
		enum UniformType
		{
			Float,
			Int,
			Vec2,
			Vec3,
			Vec4,
			Mat3,
			Mat4,
		};

		class Uniform
		{
		public:
			std::string name;
			int index;
			int size;
			UniformType type;
			int id;

			Uniform(const std::string &name, int size, UniformType type)
			{
				id = -1;
				this->name = name;
				this->size = size;
				this->type = type;
				index = 0;
			}

			template<class T>
			void set(char* data, const T& value)
			{ // quite magical that this works :D
				data[index - 1]++;
				(T&)(data[index]) = value;
			}

		};

		char* uniformData;
		char* activeUniformData;
		int uniformCount;
		int uniformSize;
		Uniform* uniforms[16];
		Shader();


		template<class T>
		void setUniformName(T value, const std::string& name, UniformType type)
		{
			Uniform* uniform = NULL;
			switch (type)
			{
			case Float:				uniform = new Uniform(name,	sizeof(float), type);	break;
			case Int:				uniform = new Uniform(name, sizeof(int), type);	break;
			case Vec2:				uniform = new Uniform(name, sizeof(float)* 2, type);	break;
			case Vec3:				uniform = new Uniform(name, sizeof(float)* 3, type);	break;
			case Vec4:				uniform = new Uniform(name, sizeof(float)* 4, type);	break;
			case Mat3:				uniform = new Uniform(name, sizeof(float)* 3 * 3, type);	break;
			case Mat4:				uniform = new Uniform(name, sizeof(float)* 4 * 4, type);	break;
			}
			assert(uniform);

			uniforms[(int)value] = uniform;
			uniformCount = glm::max(uniformCount, (int)value+1);

			uniform->index = uniformSize+1;
			uniformSize += uniform->size+1;
		}


		template <class T, class Enum>
		inline void setUniform(Enum name,			const T& value)
		{
			assert(uniformData);
			uniforms[(int)name]->set(uniformData, value);

		}
		virtual void use() = 0;
		virtual void initFromData(std::string vertexShader, std::string fragmentShader);
		virtual void bindAttributeLocation(std::string name, int index);
		void finishUniformSetup();


		virtual void setState(char* state);
	protected:
		virtual void doUniform(const std::string &name,			const glm::mat4& value) = 0;
		virtual void doUniform(const std::string &name,			const glm::mat3& value) = 0;
		virtual void doUniform(const std::string &name,			const float& value) = 0;
		virtual void doUniform(const std::string &name,			const glm::vec4& value) = 0;
		virtual void doUniform(const std::string &name,			const int& value) = 0;
		virtual void doUniform(const std::string &name,			const glm::vec3& value) = 0;
		virtual void doUniform(const std::string &name,			const glm::vec2& value) = 0;
	};
}