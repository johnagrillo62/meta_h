#include "../meta/field.h"
#include <iostream>
#include <string>
#include <vector>

struct Car {
  std::string maker = "Unknown";
  std::string model = "Base Model";
  unsigned short year = 2024;
  bool electric = false;
};

namespace meta::Car {
static constexpr auto fields = std::make_tuple(
    MakeField<&::Car::maker>("maker", Props{Prop::Serializable}, Json{"maker"},
                             Description{"Car manufacturer"}),

    MakeField<&::Car::model>("model", Props{Prop::Serializable}, Json{"model"},
                             Description{"Car model"}),

    MakeField<&::Car::year>("year", Props{Prop::Serializable}, Json{"year"},
                            Description{"Year"}),

    MakeField<&::Car::electric>("electric", Props{Prop::Serializable},
                                Json{"electric"},
                                Description{"Electric vehicle"}));
}

int main() {
  std::cout << "Clean CTAD syntax with MakeField!\n\n";

  std::cout << "Instead of:\n";
  std::cout
      << "  meta::Field<::Car, &::Car::maker, meta::Props, meta::Json>(\n";
  std::cout << "    \"maker\",\n";
  std::cout << "    meta::Props{...},\n";
  std::cout << "    meta::Json{...}\n";
  std::cout << "  )\n\n";

  std::cout << "Use:\n";
  std::cout << "  meta::MakeField<&::Car::maker>(\n";
  std::cout << "    \"maker\",\n";
  std::cout << "    meta::Props{...},\n";
  std::cout << "    meta::Json{...}\n";
  std::cout << "  )\n\n";

  std::cout << "✓ Class type deduced from member pointer\n";
  std::cout << "✓ Attribute types deduced from arguments\n";
  std::cout << "✓ Much cleaner!\n";

  // Demonstrate it works
  constexpr auto &maker_field = std::get<0>(meta::Car::fields);
  std::cout << "\nField name: " << maker_field.fieldName << "\n";
  std::cout << "Has Props: " << std::boolalpha << maker_field.has<meta::Props>()
            << "\n";
  std::cout << "Has Description: " << maker_field.has<meta::Description>()
            << "\n";

  return 0;
}
