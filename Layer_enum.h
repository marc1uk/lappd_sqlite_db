#ifndef LAYER_H
#define LAYER_H
#include <sqlite_orm.h>
// boy this is a whole lot of complex wrapping just to handle an enum
// n.b. we're gonna store it in the DB as text, because that's more user friendly
// with third party viewers or just sql queries since sqlite doesn't have internal enums

enum class Layer { Top, Middle, Bottom };

int LayerToInt(Layer layer);
std::string LayerToString(Layer layer);
std::unique_ptr<Layer> LayerFromInt(const int& layer);
std::unique_ptr<Layer> LayerFromString(const std::string& layer);

namespace sqlite_orm {
	template<>
	//struct type_printer<Layer> : public integer_printer {};
	struct type_printer<Layer> : public text_printer {};
	
	template<>
	struct statement_binder<Layer> {
		int bind(sqlite3_stmt* stmt, int index, const Layer& value) {
			//return statement_binder<int>().bind(stmt, index, LayerToInt(value));
			return statement_binder<std::string>().bind(stmt, index, LayerToString(value));
		}
	};
	
	template<>
	struct field_printer<Layer> {
		std::string operator()(const Layer& t) const {
			return LayerToString(t);
		}
	};
	
	template<>
	struct row_extractor<Layer> {
		Layer extract(const char* columnText) const {
			// note that although the accepted type is const char*, if we stored it with statement_binder<int>
			// it stored an integer, so this is a numeric text string - don't pass it to LayerFromString!
			//if (auto layer = LayerFromInt(atoi(columnText))) {
			if (auto layer = LayerFromString(columnText)) {
				return *layer;
			} else {
				throw std::runtime_error("incorrect Layer string (" + std::string(columnText) + ")");
			}
		}
		Layer extract(sqlite3_stmt* stmt, int columnIndex) const {
			auto str = sqlite3_column_text(stmt, columnIndex);
			return this->extract((const char*)str);
		}
	};
}

#endif
