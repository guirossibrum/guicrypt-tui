require 'open3'

module GuicryptTui
  class GocryptfsService
    def self.installed?
      system('which gocryptfs > /dev/null 2>&1')
    end

    def self.create(path, password)
      Dir.chdir(path) do
        Open3.popen3('gocryptfs', '-init', '.') do |stdin, stdout, stderr, wait_thr|
          stdin.puts password
          stdin.puts password  # confirm
          stdin.close
          wait_thr.value.success?
        end
      end
    end

    def self.mount(path, mount_point, password)
      Open3.popen3('gocryptfs', path, mount_point) do |stdin, stdout, stderr, wait_thr|
        stdin.puts password
        stdin.close
        wait_thr.value.success?
      end
    end

    def self.unmount(mount_point)
      system('fusermount', '-u', mount_point)
    end
  end
end