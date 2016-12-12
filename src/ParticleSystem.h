#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <GLFW/glfw3.h>
#include <utility>
#include <stdlib.h>
#include <unordered_map>
#include <map>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace std;

class fileCache
{
public:
	typedef unordered_map<string, string> StringMap;
	typedef pair<string, string> StringPair;
	static StringMap fileMap;
	static string& getFileContents(string path)
	{
		// cout << "filemap size: " << fileMap.size() << endl;
		StringMap::const_iterator got = fileMap.find(path);
		if(got == fileMap.end())
		{
			ifstream infile { path };
			string filestr { istreambuf_iterator<char>(infile), istreambuf_iterator<char>() };
			fileMap.insert(StringPair(path, filestr));
			return fileMap.find(path)->second;
		}
		else
		{	
			return fileMap.find(path)->second;
		}
	}
};

class jsonVerify
{
public:

	static bool verifyString(rapidjson::Document& doc, string name, string& result)
	{
		if(doc.HasMember(name.c_str()))
		{
			if(doc[name.c_str()].IsString())
			{
				string res = doc[name.c_str()].GetString();
				result = res;
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
				int res = doc[name.c_str()].GetInt();
				result = res;
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
				float res = doc[name.c_str()].GetDouble();
				result = res;
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
				double res = doc[name.c_str()].GetDouble();
				result = res;
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
	double last_frame_time;

	glm::vec3 position;
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
		last_frame_time = glfwGetTime();

		string jsonstr = fileCache::getFileContents(jsonpath);
		const char* jsondata = jsonstr.c_str();
		rapidjson::Document document;
		document.Parse(jsondata);

		jsonVerify::verifyDouble(document, "lifespan", lifespan);

		glm::vec3 offset;
		glm::vec3 offset_range;

		jsonVerify::verifyFloat(document, "offset_x", offset[0]);
		jsonVerify::verifyFloat(document, "offset_y", offset[1]);
		jsonVerify::verifyFloat(document, "offset_z", offset[2]);

		jsonVerify::verifyFloat(document, "offset_range_x", offset_range[0]);
		jsonVerify::verifyFloat(document, "offset_range_y", offset_range[1]);
		jsonVerify::verifyFloat(document, "offset_range_z", offset_range[2]);

		offset_range[0] = ((float(rand()) / float(RAND_MAX)) * (offset_range[0] + offset_range[0])) - offset_range[0];
    	// cout << "\noffrange0 = " << offset_range[0];

    	offset_range[1] = ((float(rand()) / float(RAND_MAX)) * (offset_range[1] + offset_range[1])) - offset_range[1];
    	// cout << "\noffrange1 = " << offset_range[1];

    	offset_range[2] = ((float(rand()) / float(RAND_MAX)) * (offset_range[2] + offset_range[2])) - offset_range[2];
    	// cout << "\noffrange2 = " << offset_range[2];

    	position = position + offset + offset_range;

    	jsonVerify::verifyFloat(document, "start_velocity_x", velocity[0]);
    	jsonVerify::verifyFloat(document, "start_velocity_y", velocity[1]);
    	jsonVerify::verifyFloat(document, "start_velocity_z", velocity[2]);

    	jsonVerify::verifyFloat(document, "acceleration_x", acceleration[0]);
    	jsonVerify::verifyFloat(document, "acceleration_y", acceleration[1]);
    	jsonVerify::verifyFloat(document, "acceleration_z", acceleration[2]);


    	jsonVerify::verifyFloat(document, "start_alpha", start_alpha);
		alpha = start_alpha;
		jsonVerify::verifyFloat(document, "end_alpha", end_alpha);jsonVerify::verifyFloat(document, "start_alpha", start_alpha);
		alpha = start_alpha;
		jsonVerify::verifyFloat(document, "end_alpha", end_alpha);

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
		float dtime = glfwGetTime() - last_frame_time;
		velocity += dtime * acceleration;
		position += dtime * velocity;
		double time_diff = glfwGetTime() - timespawned;
		float percent = time_diff / lifespan;
		color.x = start_color.x + percent*(end_color.x - start_color.x);
		color.y = start_color.y + percent*(end_color.y - start_color.y);
		color.z = start_color.z + percent*(end_color.z - start_color.z);
		alpha = start_alpha + percent*(end_alpha - start_alpha);
		scale = start_scale + percent*(end_scale - start_scale);
		rotation = start_rotation + percent*(end_rotation - start_rotation);
		last_frame_time = glfwGetTime();
	}
};

struct ParticleEmitter: ParticleEntity
{
	string emission_entity_path;
	vector<ParticleEmitter>* active_emitters;
	vector<ParticleBillboard>* active_billboards;
	
	double emission_rate;
	double last_emit_time;
	double spawn_delay;

	void ConfigDefault(string jsonpath)
	{
		isBillboard = false;
		timespawned = glfwGetTime();
		last_frame_time = glfwGetTime();
		last_emit_time = -1.0;

		string jsonstr = fileCache::getFileContents(jsonpath);
		const char* jsondata = jsonstr.c_str();
		rapidjson::Document document;
		document.Parse(jsondata);

		jsonVerify::verifyDouble(document, "lifespan", lifespan);

		glm::vec3 offset;
		glm::vec3 offset_range;

		jsonVerify::verifyFloat(document, "offset_x", offset[0]);
		jsonVerify::verifyFloat(document, "offset_y", offset[1]);
		jsonVerify::verifyFloat(document, "offset_z", offset[2]);

		jsonVerify::verifyFloat(document, "offset_range_x", offset_range[0]);
		jsonVerify::verifyFloat(document, "offset_range_y", offset_range[1]);
		jsonVerify::verifyFloat(document, "offset_range_z", offset_range[2]);

    	offset_range[0] = ((float(rand()) / float(RAND_MAX)) * (offset_range[0] + offset_range[0])) - offset_range[0];
    	// cout << "\noffrange0 = " << offset_range[0];

    	offset_range[1] = ((float(rand()) / float(RAND_MAX)) * (offset_range[1] + offset_range[1])) - offset_range[1];
    	// cout << "\noffrange1 = " << offset_range[1];

    	offset_range[2] = ((float(rand()) / float(RAND_MAX)) * (offset_range[2] + offset_range[2])) - offset_range[2];
    	// cout << "\noffrange2 = " << offset_range[2];

    	position = position + offset + offset_range;

    	jsonVerify::verifyFloat(document, "start_velocity_x", velocity[0]);
    	jsonVerify::verifyFloat(document, "start_velocity_y", velocity[1]);
    	jsonVerify::verifyFloat(document, "start_velocity_z", velocity[2]);

    	jsonVerify::verifyFloat(document, "acceleration_x", acceleration[0]);
    	jsonVerify::verifyFloat(document, "acceleration_y", acceleration[1]);
    	jsonVerify::verifyFloat(document, "acceleration_z", acceleration[2]);

		jsonVerify::verifyString(document, "emission_entity", emission_entity_path);

		jsonVerify::verifyDouble(document, "emission_rate", emission_rate);

		jsonVerify::verifyDouble(document, "spawn_delay", spawn_delay);
	}

	void Emit()
	{
		string jsonstr = fileCache::getFileContents(emission_entity_path);
		const char* jsondata = jsonstr.c_str();
		rapidjson::Document document;
		document.Parse(jsondata);

		string type;
		jsonVerify::verifyString(document, "type", type);

		if(type == "emitter")
		{
			ParticleEmitter e;
			e.active_emitters = active_emitters;
			e.active_billboards = active_billboards;
			e.position = position;
			e.ConfigDefault(emission_entity_path);
			active_emitters->push_back(e);
		}
		else if(type == "billboard")
		{
			ParticleBillboard b;
			b.position = position;
			b.ConfigDefault(emission_entity_path);
			active_billboards->push_back(b);
		}
		else
		{
			cout << "Emission entity read from " << emission_entity_path << " is defined as neither emitter nor billboard. Cannot emit." << endl;
			return;
		}
		last_emit_time = glfwGetTime();
	}

	void Update()
	{
		float dtime = glfwGetTime() - last_frame_time;
		velocity += dtime * acceleration;
		position += dtime * velocity;
		if(last_emit_time == -1.0)
		{
			if(glfwGetTime() - spawn_delay >= (1.0/emission_rate))
			{
				Emit();
			}
		}	
		else
		{
			if(glfwGetTime() - last_emit_time >= (1.0/emission_rate))
			{
				Emit();
			}
		}
	}
};

struct ParticleSystem
{
	vector<ParticleEmitter> active_emitters;
	vector<ParticleBillboard> active_billboards;
	vector<string> emitter_paths;
	glm::vec3 position;
	bool is_finished;

	void ConfigDefault(string jsonpath)
	{
		is_finished = false;
		string jsonstr = fileCache::getFileContents(jsonpath);
		// std::cout<<"\n\n\nfilepath: "<<jsonstr<<"\n\n\n";
		const char* jsondata = jsonstr.c_str();
		rapidjson::Document document;
		document.Parse(jsondata);

		jsonVerify::verifyFloat(document, "position_x", position[0]);
		jsonVerify::verifyFloat(document, "position_y", position[1]);
		jsonVerify::verifyFloat(document, "position_z", position[2]);

		const rapidjson::Value& emitters = document["emitters"];
		if(emitters.IsArray())
		{
			for (rapidjson::SizeType i = 0; i < emitters.Size(); i++)
			{
				ParticleEmitter e;
				e.active_emitters = &active_emitters;
				e.active_billboards = &active_billboards;
				e.ConfigDefault(emitters[i].GetString());
				active_emitters.push_back(e);
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
		for(int i = 0; i < active_emitters.size(); i++)
		{
			if(active_emitters[i].lifespan != -1 && (glfwGetTime() - active_emitters[i].timespawned) >= active_emitters[i].lifespan)
			{
				active_emitters.erase(active_emitters.begin()+i);
			}
			else
			{
				active_emitters[i].Update();
			}
			// std::cout<<"\nactive_entities size: "<<active_entities.size();
		}
		for(int i = 0; i < active_billboards.size(); i++)
		{
			if(active_billboards[i].lifespan != -1 && (glfwGetTime() - active_billboards[i].timespawned) >= active_billboards[i].lifespan)
			{
				active_billboards.erase(active_billboards.begin()+i);
			}
			else
			{
				active_billboards[i].Update();
			}
			// std::cout<<"\nactive_entities size: "<<active_entities.size();
		}
		is_finished = (active_emitters.size() > 0 || active_billboards.size() > 0);
			// std::cout<<"\nactive_entities size: "<<active_entities.size();
		
	}
};