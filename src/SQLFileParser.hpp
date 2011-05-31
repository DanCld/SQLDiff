#ifndef SQLFILEPARSER_HPP
#define SQLFILEPARSER_HPP

/* Dan-Claudiu Dragos <dancld@yahoo.co.uk>
* License: GPL
*/

namespace sqlfileparser
{

class TextScannerHelper
{
	public:

		TextScannerHelper();

		const std::string& buffer() const { return buffer_; }

		void addToBuffer(const char* piece);

		void resetBuffer();

	private:

		std::string buffer_;

};

} // namespace

#endif
