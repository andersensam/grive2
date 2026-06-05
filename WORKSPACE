workspace(name = "grive2")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "rules_foreign_cc",
    sha256 = "6041fce7e231610c1304bbc612f81c7abd8b5cd698e076717d06a26d246252ce",
    strip_prefix = "rules_foreign_cc-0.13.0",
    url = "https://github.com/bazelbuild/rules_foreign_cc/releases/download/0.13.0/rules_foreign_cc-0.13.0.tar.gz",
)

load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")

rules_foreign_cc_dependencies()

http_archive(
    name = "yajl_src",
    strip_prefix = "yajl-2.1.0",
    urls = ["https://github.com/lloyd/yajl/archive/refs/tags/2.1.0.tar.gz"],
    build_file_content = """filegroup(name="all_srcs", srcs=glob(["**"]), visibility=["//visibility:public"])""",
)

http_archive(
    name = "libgpg_error_src",
    strip_prefix = "libgpg-error-1.50",
    urls = ["https://www.gnupg.org/ftp/gcrypt/libgpg-error/libgpg-error-1.50.tar.bz2"],
    build_file_content = """filegroup(name="all_srcs", srcs=glob(["**"]), visibility=["//visibility:public"])""",
)

http_archive(
    name = "libgcrypt_src",
    strip_prefix = "libgcrypt-1.11.0",
    urls = ["https://www.gnupg.org/ftp/gcrypt/libgcrypt/libgcrypt-1.11.0.tar.bz2"],
    build_file_content = """filegroup(name="all_srcs", srcs=glob(["**"]), visibility=["//visibility:public"])""",
)

http_archive(
    name = "cppunit_src",
    sha256 = "89c5c6665337f56fd2db36bc3805a5619709d51fb136e51937072f63fcc717a7",
    strip_prefix = "cppunit-1.15.1",
    urls = ["https://dev-www.libreoffice.org/src/cppunit-1.15.1.tar.gz"],
    build_file_content = """filegroup(name="all_srcs", srcs=glob(["**"]), visibility=["//visibility:public"])""",
)
