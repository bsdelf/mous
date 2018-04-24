class Mous < Formula
  desc "Simple yet powerful audio player for FreeBSD/Linux/macOS"
  homepage "https://github.com/bsdelf/mous"
  url "https://github.com/bsdelf/mous/archive/v2.0.1.tar.gz"
  sha256 "b8866049c225fab2033617908d3c3b353533bc20c274b1fa0d7c034ea1c15cef"

  head "https://github.com/bsdelf/mous.git"

  depends_on "cmake" => :build
  depends_on "taglib" => :recommended
  depends_on "libcue" => :recommended
  depends_on "fdk-aac" => :recommended
  depends_on "mpg123" => :recommended
  depends_on "flac" => :recommended
  depends_on "libogg" => :recommended
  depends_on "wavpack" => :recommended
  depends_on "qt" => :optional

  def install
    mkdir "build" do
      system "cmake", "..", *std_cmake_args
      system "make", "install"
    end
  end

  test do
    assert_predicate lib/"libMousCore.dylib", :exist?
  end
end
