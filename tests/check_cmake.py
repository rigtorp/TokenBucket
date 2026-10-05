#!/usr/bin/env python3
"""Check embedded, build-tree, installed, and relocated CMake consumers."""

import argparse
from pathlib import Path
import subprocess
import tempfile


def run(*command):
    print('+', ' '.join(map(str, command)), flush=True)
    subprocess.run(list(map(str, command)), check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config', default='Debug')
    parser.add_argument('--standard', default='11', choices=['11', '17'])
    args = parser.parse_args()
    source = Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix='tokenbucket-cmake-') as temporary:
        root = Path(temporary)
        common = ['-DCMAKE_BUILD_TYPE=' + args.config,
                  '-DCMAKE_CXX_STANDARD=' + args.standard]
        consumer = root / 'consumer'
        consumer.mkdir()
        (consumer / 'main.cpp').write_text(
            '#include <rigtorp/TokenBucket.h>\n'
            'int main() { rigtorp::TokenBucket<> b(1, 1); '
            'return b.consume(1) ? 0 : 1; }\n')
        (consumer / 'CMakeLists.txt').write_text(
            'cmake_minimum_required(VERSION 3.20)\n'
            'project(Consumer LANGUAGES CXX)\n'
            'if(BUCKET_SOURCE)\n'
            '  add_subdirectory("${BUCKET_SOURCE}" bucket)\n'
            'else()\n'
            '  find_package(TokenBucket 1 REQUIRED CONFIG)\n'
            'endif()\n'
            'add_executable(consumer main.cpp)\n'
            'target_link_libraries(consumer PRIVATE TokenBucket::TokenBucket)\n'
            'enable_testing()\n'
            'add_test(NAME consumer COMMAND consumer)\n'
            'set_tests_properties(consumer PROPERTIES TIMEOUT 10)\n')

        def check(name, *options):
            build = root / (name + '-consumer')
            run('cmake', '-S', consumer, '-B', build, *common, *options)
            run('cmake', '--build', build, '--config', args.config)
            run('ctest', '--test-dir', build, '-C', args.config,
                '--output-on-failure', '--no-tests=error')

        check('embedded', '-DBUCKET_SOURCE=' + source.as_posix())
        # Embedding should neither build library tests nor add install rules.
        assert not list((root / 'embedded-consumer').rglob('TokenBucketTest*'))
        embedded_install = (root / 'embedded-consumer/bucket/cmake_install.cmake').read_text()
        assert 'TokenBucketTargets' not in embedded_install

        for name, includedir, libdir in [('default', 'include', 'lib'),
                                         ('custom', 'headers', 'packages')]:
            build, prefix = root / (name + '-build'), root / (name + '-install')
            run('cmake', '-S', source, '-B', build, *common,
                '-DTOKENBUCKET_BUILD_TESTS=OFF',
                '-DCMAKE_INSTALL_PREFIX=' + prefix.as_posix(),
                '-DCMAKE_INSTALL_INCLUDEDIR=' + includedir,
                '-DCMAKE_INSTALL_LIBDIR=' + libdir)
            check(name + '-build-tree', '-DTokenBucket_DIR=' + build.as_posix())
            run('cmake', '--install', build, '--config', args.config)
            check(name + '-installed', '-DTokenBucket_DIR=' +
                  (prefix / libdir / 'cmake/TokenBucket').as_posix())
            relocated = root / (name + '-relocated')
            prefix.rename(relocated)
            check(name + '-relocated', '-DTokenBucket_DIR=' +
                  (relocated / libdir / 'cmake/TokenBucket').as_posix())

        # An embedded parent can explicitly request the package's install rules.
        check('embedded-install', '-DBUCKET_SOURCE=' + source.as_posix(),
              '-DTOKENBUCKET_INSTALL=ON')
        prefix = root / 'embedded-install-prefix'
        run('cmake', '--install', root / 'embedded-install-consumer',
            '--config', args.config, '--prefix', prefix)
        package = next(prefix.rglob('TokenBucketConfig.cmake')).parent
        check('embedded-installed', '-DTokenBucket_DIR=' + package.as_posix())


if __name__ == '__main__':
    main()
