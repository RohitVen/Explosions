#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <time.h>
#include <streambuf>
#include <GLFW/glfw3.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace std;

class jsonVerify
{
public:
	static bool verifyString(rapidjson::Document& doc, string name, string& result)
	{
		if(doc.HasMember(name.c_str()))
		{
			if(doc[name.c_str()].IsString())
			{
				result = doc[name.c_str()].GetString();
				return true;
			}
			else
			{
				cout << "Value " << name << " in json file isn't a string!" << endl;
			}
		}
		else
		{
			cout << "Json file has no such member " << name << endl;
		}
		return false;
	}

	static bool verifyInt(rapidjson::Document& doc, string name, int& result)
	{
		if(doc.HasMember(name.c_str()))
		{
			if(doc[name.c_str()].IsInt())
			{
				result = doc[name.c_str()].GetInt();
				return true;
			}
			else
			{
				cout << "Value " << name << " in json file isn't an int!" << endl;
			}
		}
		else
		{
			cout << "Json file has no such member " << name << endl;
		}
		return false;
	}

	static bool verifyFloat(rapidjson::Document& doc, string name, float& result)
	{
		if(doc.HasMember(name.c_str()))
		{
			if(doc[name.c_str()].IsDouble())
			{
				result = doc[name.c_str()].GetDouble();
				return true;
			}
			else
			{
				cout << "Value " << name << " in json file isn't a double!" << endl;
			}
		}
		else
		{
			cout << "Json file has no such member " << name << endl;
		}
		return false;
	}

	static bool verifyDouble(rapidjson::Document& doc, string name, double& result)
	{
		if(doc.HasMember(name.c_str()))
		{
			if(doc[name.c_str()].IsDouble())
			{
				result = doc[name.c_str()].GetDouble();
				return true;
			}
			else
			{
				cout << "Value " << name << " in json file isn't a double!" << endl;
			}
		}
		else
		{
			cout << "Json file has no such member " << name << endl;
		}
		return false;
	}
};

struct ParticleEntity
{
	bool isBillboard;
	double timespawned;
	double lifespan = 1.0;

	glm::vec3 offset;
	glm::vec3 offset_range;
	glm::vec3 acceleration;
	glm::vec3 velocity;

	virtual void ConfigDefault(string jsonpath) {};
	virtual void Update() {};
};

struct ParticleBillboard: ParticleEntity
{
	string texture_path;

	glm::vec3 color;
	glm::vec3 start_color;
	glm::vec3 end_color;

	float alpha;
	float start_alpha;
	float end_alpha;

	float rotation;
	float start_rotation;
	float end_rotation;

	float scale;
	float start_scale;
	float end_scale;

	void ConfigDefault(string jsonpath)
	{
		isBillboard = true;
		timespawned = glfwGetTime();

		ifstream infile { jsonpath };
		string jsonstr { istreambuf_iterator<char>(infile), istreambuf_iterator<char>() };
		const char* jsondata = jsonstr.c_str();
		rapidjson::Document document;
		document.Parse(jsondata);

		jsonVerify::verifyDouble(document, "lifespan", lifespan);

		jsonVerify::verifyFloat(document, "offset_x", offset[0]);
		jsonVerify::verifyFloat(document, "offset_y", offset[1]);
		jsonVerify::verifyFloat(document, "offset_z", offset[2]);

		jsonVerify::verifyFloat(document, "offset_range_x", offset_range[0]);
		jsonVerify::verifyFloat(document, "offset_range_y", offset_range[1]);
		jsonVerify::verifyFloat(document, "offset_range_z", offset_range[2]);

		jsonVerify::verifyFloat(document, "start_color_r", start_color[0]);
		jsonVerify::verifyFloat(document, "start_color_g", start_color[1]);
		jsonVerify::verifyFloat(document, "start_color_b", start_color[2]);

		color = start_color;

		jsonVerify::verifyFloat(document, "end_color_r", end_color[0]);
		jsonVerify::verifyFloat(document, "end_color_g", end_color[1]);
		jsonVerify::verifyFloat(document, "end_color_b", end_color[2]);

		jsonVerify::verifyFloat(document, "start_rotation", start_rotation);
		rotation = start_rotation;
		jsonVerify::verifyFloat(document, "end_rotation", end_rotation);

		jsonVerify::verifyFloat(document, "start_scale", start_scale);
		scale = start_scale;
		jsonVerify::verifyFloat(document, "end_scale", end_scale);

		jsonVerify::verifyFloat(document, "start_alpha", start_alpha);
		alpha = start_alpha;
		jsonVerify::verifyFloat(document, "end_alpha", end_alpha);
	}

	void Update()
	{
		double time_diff = glfwGetTime() - timespawned;
		float percent = time_diff / lifespan;
		if(color.x < end_color.x && color.y < end_color.y && color.z < end_color.z)
			color += glm::vec3(percent, percent, percent);
		if(scale < end_scale)
			scale += percent;
		if(rotation < end_rotation)
			rotation += percent;
		if(alpha < end_alpha)
			alpha += percent;
	}
};

struct ParticleEmitter: ParticleEntity
{
	string emission_entity_path;
	vector<ParticleEntity>* active_entities;
	double emission_rate;
	double last_emit_time;
	double spawn_offset;

	void ConfigDefault(string jsonpath)
	{
		isBillboard = false;
		timespawned = glfwGetTime();
		last_emit_time = -1.0;

		ifstream infile { jsonpath };
		string jsonstr { istreambuf_iterator<char>(infile), istreambuf_iterator<char>() };
		const char* jsondata = jsonstr.c_str();
		rapidjson::Document document;
		document.Parse(jsondata);

		jsonVerify::verifyDouble(document, "lifespan", lifespan);

		jsonVerify::verifyFloat(document, "offset_x", offset[0]);
		jsonVerify::verifyFloat(document, "offset_y", offset[1]);
		jsonVerify::verifyFloat(document, "offset_z", offset[2]);

		jsonVerify::verifyFloat(document, "offset_range_x", offset_range[0]);
		jsonVerify::verifyFloat(document, "offset_range_y", offset_range[1]);
		jsonVerify::verifyFloat(document, "offset_range_z", offset_range[2]);

		jsonVerify::verifyString(document, "emission_entity", emission_entity_path);

		jsonVerify::verifyDouble(document, "emission_rate", emission_rate);

		jsonVerify::verifyDouble(document, "spawn_offset", spawn_offset);
	}

	void Emit()
	{
		ifstream infile { emission_entity_path };
		string jsonstr { istreambuf_iterator<char>(infile), istreambuf_iterator<char>() };
		const char* jsondata = jsonstr.c_str();
		rapidjson::Document document;
		document.Parse(jsondata);

		string type;
		jsonVerify::verifyString(document, "type", type);

		if(type == "emitter")
		{
			ParticleEmitter e;
			e.active_entities = active_entities;
			e.ConfigDefault(emission_entity_path);
			active_entities->push_back(e);
		}
		else if(type == "billboard")
		{
			ParticleBillboard b;
			b.ConfigDefault(emission_entity_path);
			active_entities->push_back(b);
		}
		else
		{
			cout << "Emission entity read from " << emission_entity_path << " is defined as neither emitter nor billboard. Cannot emit." << endl;
			return;
		}
	}
};

struct ParticleSystem
{
	vector<ParticleEntity> active_entities;
	vector<string> emitter_paths;
	glm::vec3 position;
	double lifespan = 1.0;

	void ConfigDefault(string jsonpath)
	{
		ifstream infile { jsonpath };
		if(!infile)
		{
			std::cout<<"\nERROR: Unable to find specified file";
		}
		string jsonstr { istreambuf_iterator<char>(infile), istreambuf_iterator<char>() };
		// std::cout<<"\n\n\nfilepath: "<<jsonstr<<"\n\n\n";
		const char* jsondata = jsonstr.c_str();
		rapidjson::Document document;
		document.Parse(jsondata);

		position[0] = document["position_x"].GetDouble();
		position[1] = document["position_y"].GetDouble();
		position[2] = document["position_z"].GetDouble();
		lifespan = document["lifespan"].GetDouble();
		const rapidjson::Value& emitters = document["emitters"];
		if(emitters.IsArray())
		{
			for (rapidjson::SizeType i = 0; i < emitters.Size(); i++)
			{
				ParticleEmitter e;
				e.active_entities = &active_entities;
				e.ConfigDefault(emitters[i].GetString());
				active_entities.push_back(e);
			}
		}
		else
		{
			cout << "Value 'emitters' in "<< jsonpath << " must be an array type! Cannot set up emitters." << endl;
		}
		std::cout<<"\nemitter_paths: "<<emitter_paths.size();
	}

	void Update()
	{
		for(int i = 0; i < active_entities.size(); i++)
		{
			if((glfwGetTime() - active_entities[i].timespawned) >= active_entities[i].lifespan)
			{
				active_entities.erase(active_entities.begin()+i);
			}
			active_entities[i].Update();
			// std::cout<<"\nactive_entities size: "<<active_entities.size();
		}
			// std::cout<<"\nactive_entities size: "<<active_entities.size();
		
	}
};