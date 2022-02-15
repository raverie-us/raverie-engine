// MIT Licensed (see LICENSE.md).
namespace Zero
{

// Simple binary com port reader.
class ComPort
{
public:
  ComPort();
  ~ComPort();

  bool Open(StringParam name, uint baudRate);
  void Close();
  uint Read(byte* buffer, uint bytesToRead);
  void Write(byte* buffer, uint bytesToWrite);

  OsHandle mComHandle;
};

} // namespace Zero
