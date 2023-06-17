/**
 * @file
 *
 * Header file for declaring the Blob class.
 */

#ifndef GHOTI_WAVE_BLOB_HPP
#define GHOTI_WAVE_BLOB_HPP

#include <ghoti.io/shared_string_view.hpp>
#include <ghoti.io/os/file.hpp>

namespace Ghoti::Wave {

/**
 */
class Blob {
  public:
  enum class Type {
    TEXT,
    FILE,
  };

  /**
   * The default constructor.
   */
  Blob();

  /**
   * Construct a Blob with a text representation.
   *
   * @param text The text the blob should contain.
   */
  Blob(const Ghoti::shared_string_view & text);

  /**
   * Construct a Blob with a file representation.
   *
   * @param file The file the blob should contain.
   */
  Blob(Ghoti::OS::File && file);

  /**
   * Set the text contents of the Blob.
   *
   * This will replace any current contents, whether file or text.
   *
   * @param text The text the blob should contain.
   */
  void set(Ghoti::shared_string_view & text);

  /**
   * Set the file contents of the Blob.
   *
   * This will replace any current contents, whether file or text.
   *
   * @param file The file the blob should contain.
   */
  void set(Ghoti::OS::File && file);

  /**
   * Get the text in the blob.
   *
   * If the Blob is a file blob, then the text will be empty.
   *
   * @return The text in the blob.
   */
  const Ghoti::shared_string_view & getText() const;

  /**
   * Get the file in the blob.
   *
   * If the Blob is a text blob, then the file will be empty.
   *
   * @return The file in the blob.
   */
  const Ghoti::OS::File & getFile() const;

  /**
   * Get the Ghoti::Wave::Blob::Type of data the blob contains.
   *
   * @return The Ghoti::Wave::Blob::Type of data the blob contains.
   */
  Ghoti::Wave::Blob::Type getType() const;

  /**
   * Compare a file against a string.
   *
   * @param rhs The string to compare against.
   * @return True if the values are equivalent, False otherwise.
   */
  bool operator==(const Ghoti::shared_string_view & rhs) const;

  private:
  /**
   * The type of data the blob contains.
   */
  Ghoti::Wave::Blob::Type type;

  /**
   * The text data the blob contains.
   */
  Ghoti::shared_string_view text;

  /**
   * The file data the blob contains.
   */
  Ghoti::OS::File file;
};

/**
 * Helper function to output a Blob to a stream.
 *
 * @param out The output stream.
 * @param blob The Blob to be inserted into the stream.
 *
 * @return The output stream.
 */
std::ostream & operator<<(std::ostream & out, const Blob & blob);

}

#endif // GHOTI_WAVE_BLOB_HPP

