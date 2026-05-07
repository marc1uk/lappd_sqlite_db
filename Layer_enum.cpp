#include "Layer_enum.h"
#include <iostream>

int LayerToInt(Layer layer) { return int(layer); }

std::string LayerToString(Layer layer) {
	int l = static_cast<int>(layer);
	switch(l){
		case 0: return "Top";
		case 1: return "Middle";
		case 2: return "Bottom";
	}
	throw std::domain_error("Invalid Layer enum");
}

std::unique_ptr<Layer> LayerFromInt(const int& layer) {
	switch(layer){
		case 0: return std::make_unique<Layer>(Layer::Top);
		case 1: return std::make_unique<Layer>(Layer::Middle);
		case 2: return std::make_unique<Layer>(Layer::Bottom);
	}
	return nullptr;
}

std::unique_ptr<Layer> LayerFromString(const std::string& layer) {
	if(layer=="Top") return std::make_unique<Layer>(Layer::Top);
	if(layer=="Middle") return std::make_unique<Layer>(Layer::Middle);
	if(layer=="Bottom") return std::make_unique<Layer>(Layer::Bottom);
	return nullptr;
}

