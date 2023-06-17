/**
 * @file
 *
 * Define the Ghoti::Wave::Blob class.
 */

#include "blob.hpp"

using namespace std;
using namespace Ghoti::Wave;

Blob::Blob() : type{Blob::Type::TEXT}, text{}, file{} {}

Blob::Blob(const Ghoti::shared_string_view & text) : type{Blob::Type::TEXT}, text{text}, file{} {}

Blob::Blob(Ghoti::OS::File && file) : type{Blob::Type::FILE}, text{}, file{move(file)} {}

void Blob::set(Ghoti::shared_string_view & text) {
  this->text = text;
  this->type = Blob::Type::TEXT;
  this->file = {};
}

void Blob::set(Ghoti::OS::File && file) {
  this->text = {};
  this->file = move(file);
  this->type = Blob::Type::FILE;
}

const Ghoti::shared_string_view & Blob::getText() const {
  return this->text;
}

const Ghoti::OS::File & Blob::getFile() const {
  return this->file;
}

Blob::Type Blob::getType() const {
  return this->type;
}

bool Blob::operator==(const Ghoti::shared_string_view & rhs) const {
  if (this->type == Blob::Type::TEXT) {
    return this->text == rhs;
  }
  return string{this->file} == rhs;
}

error_code Blob::append(const Ghoti::shared_string_view & text) {
  if (this->type == Blob::Type::TEXT) {
    this->text += text;
    return {};
  }
  return this->file.append(text);
}

error_code Blob::truncate(const Ghoti::shared_string_view & text) {
  if (this->type == Blob::Type::TEXT) {
    this->text = text;
    return {};
  }
  return this->file.truncate(text);
}

error_code Blob::convertToFile() {
  if (this->type == Blob::Type::FILE) {
    return {};
  }

  // Attempt to create a temporary file.
  this->file = Ghoti::OS::File::createTemp("");
  error_code ec{};
  if ((ec = this->file.test())) {
    return ec;
  }

  ec = this->file.truncate(this->text);

  // Only switch the type if there have been no errors so far.
  if (!ec) {
    this->type = Blob::Type::FILE;
  }
  return ec;
}

ostream & Ghoti::Wave::operator<<(ostream & out, const Blob & blob) {
  if (blob.getType() == Blob::Type::TEXT) {
    out << blob.getText();
  }
  else {
    out << string{blob.getFile()};
  }
  return out;
}

