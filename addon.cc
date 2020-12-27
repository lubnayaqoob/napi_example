#include <napi.h>
#include <vector>
#include<stdio.h>
#include<stdlib.h>

// Define the type can_frame (It's not necessary in your case you already have this type)
struct can_frame {
	int can_dlc;
    int can_id;
	int* data;
};

// Create an alisa for the prototype callback
typedef void (*fn)(struct can_frame frame);

void startCANReceive(const char* dev, void (*fn) (struct can_frame frame)) {
	// Here I'm emulating what happen in startCANReceive function
	static const int len = 10;
	printf("Length: %d\n", len);
	int data[len];
	for (int i = 0; i < len; i++) {
		data[i] = i + 1;
	}
	struct can_frame frame{};
	frame.can_dlc = len;
	frame.can_id = 97;
	frame.data = data;
	// Call the callback
	fn(frame);	
}

/* void printCANFrame(struct can_frame frame) {
	unsigned char len = frame.can_dlc;
	printf("%X:\t[%d]\t", frame.can_id, len);
	for (int i=0;i<len;i++) {
	  printf("%0X ",frame.data[i]);
 	}
} */

struct Result {
	int len;
	int id;
	std::vector<int> buffer{};
};

struct WrapFrame {
	Result* get_result() {
		return &result;
	}
	fn operator()() {
		static auto ref = &result;
		return [](struct can_frame frame) -> void {
			int len = frame.can_dlc;
			for (int i=0;i<len;i++) {
				ref->buffer.push_back(frame.data[i]);
 			}
			ref->len = frame.can_dlc;
			ref->id = frame.can_id;
		};
	}
	Result result{};
};

void NapiCanrcvFunction(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  std::string dev = info[0].As<Napi::String>().Utf8Value();
  Napi::Function cb = info[1].As<Napi::Function>();
  WrapFrame frame{};
  startCANReceive(dev.c_str(), frame());
  Result* result = frame.get_result();
  Napi::Object data = Napi::Object::New(env); 
  data["len"] = result->len;
  data["id"] = result->id;
  // Napi::Buffer<int> buffer = Napi::Buffer<int>::New(env, result->buffer.data(), result->buffer.size());
  Napi::Array array = Napi::Array::New(env, result->buffer.size());
  for (int i = 0; i < result->buffer.size(); i++) {
	  array.Set(i, Napi::Number::New(env, result->buffer[i]));
  }
  data["buffer"] = array;
  cb.Call({ env.Null(), data});
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "NapiCanrcvFunction"),
              Napi::Function::New(env, NapiCanrcvFunction));
  return exports;
}

NODE_API_MODULE(addon, Init)
