#include <iostream>
#include <iomanip>
#include <sstream>

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/mman.h>
#include <fcntl.h>

#define PAGE_SHIFT (12)
#define PAGE_SIZE  (1UL << PAGE_SHIFT)
#define PFN_UP(x)  (((x) + PAGE_SIZE - 1) >> PAGE_SHIFT)
#define ALIGNED_SIZE(x) (PFN_UP(x) * PAGE_SIZE)

#define DEFAULT_GRAN (8)

class MemDump {
  public:
    MemDump () { gran_ = DEFAULT_GRAN; }

    int init (std::string addr_str, std::string size_str) {
      std::istringstream addr_ss (addr_str);
      std::istringstream size_ss (size_str);

      addr_ss >> std::hex >> addr_;
      size_ss >> std::hex >> size_;

      if (addr_ss.fail () || size_ss.fail ()) {
        std::cerr << "Unable to parse arguments\n";
        return -EINVAL;
      }

      if (addr_ % PAGE_SIZE != 0) {
        std::cerr << "Address should be PAGE_SIZE aligned\n";
        return -EINVAL;
      }

      std::cerr << "Addr: 0x" << std::hex << addr_ << "\n";
      std::cerr << "Size: 0x" << std::hex << size_ << "\n\n";

      return 0;
    }

    int show () {
      int fd = open ("/dev/mem", O_RDONLY | O_SYNC);
      if (fd < 0) {
        std::cerr << strerror (errno) << "\n";
        return -errno;
      }

      void *addr = mmap (0, ALIGNED_SIZE (size_), PROT_READ,
          MAP_SHARED, fd, addr_);
      if (addr == MAP_FAILED) {
        std::cerr << strerror (errno) << "\n";
        return -errno;
      }

      if (path_.empty ()) {
        print_hex_dump (static_cast<char *>(addr), size_);
      } else {
        FILE *fp = fopen (path_.c_str(), "wb");
        if (fp) {
          for (size_t i = 0; i < size_; i += gran_)
            fwrite (static_cast<char *>(addr) + i, gran_, 1, fp);
          fclose (fp);
        } else {
          std::cerr << strerror (errno) << "\n";
        }
      }

      munmap (addr, ALIGNED_SIZE (size_));
      close (fd);

      return 0;
    }

    void set_granularity (std::string gran) {
      gran_ = std::stoll (gran);
    }

    void set_file_path (std::string path) {
      path_ = path;
    }

    void print_hex_dump (char *addr, size_t size) {
      size_t idx = 0;

      std::cout << std::hex << std::setfill('0');
      while (idx < size) {
        if (addr_ <= UINT32_MAX)
          std::cout << std::setw(8) << addr_;
        else
          std::cout << addr_;

        for (size_t i = 0; i < 16; i++) {
          if (i % 8 == 0) std::cout << ' ';
          std::cout << ' ' << std::setw(2) << (unsigned int)(unsigned char)(addr[idx + i]);
        }
        std::cout << "\n";

        idx += 16;
        addr_ += 16;
      }
    }

  private:
    unsigned long addr_;
    unsigned long size_;
    size_t gran_;

    std::string path_;
};

void print_usage (const char *path)
{
  std::cerr << "Usage: " << path << " [options] <hex:addr> <hex:size>\n";
  std::cerr << "Options\n";
  std::cerr << "  -f <arg>\tSet filepath for memory dump\n";
  std::cerr << "  -g <arg>\tSet data granularity for memory dump\n";
  std::cerr << "  -h\t\tShow the usage of this program\n";
}

int main (int argc, char **argv)
{
  MemDump dump;
  int c;

  optind = 0;
  opterr = 0;
  while ((c = getopt (argc, argv, "hg:f:")) != -1) {
    switch (c) {
      case 'f':
        dump.set_file_path (optarg);
        break;
      case 'g':
        dump.set_granularity (optarg);
        break;
      case '?':
        if (optopt == 'f')
          std::cerr << "Error: Option -f requires an extra argument\n\n";
        else
          std::cerr << "Error: Unknown option " << c << "\n\n";
      case 'h':
        print_usage (argv[0]);
        return 0;
    }
  }

  if (optind + 1 >= argc) {
    std::cerr << "Error: Invalid argument provided\n";
    print_usage (argv[0]);
    return -EINVAL;
  }

  int status = dump.init (argv[optind], argv[optind + 1]);
  if (status != 0) {
    std::cerr << "Error: Fail to dump memory\n";
    return status;
  }

  return dump.show ();
}
