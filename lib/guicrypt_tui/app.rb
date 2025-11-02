require_relative 'core/vault_store'
require_relative 'core/keyring_service'
require_relative 'core/mount_service'

module GuicryptTui
  class App
    def self.run
      if ARGV.empty?
        puts "guicrypt-tui starting..."
        # TODO: Initialize UI
      else
        handle_command
      end
    end

    def self.handle_command
      command = ARGV.shift
      case command
      when 'add'
        add_vault(ARGV[0], ARGV[1])
      when 'mount'
        mount_vault(ARGV[0].to_i)
      when 'unmount'
        unmount_vault(ARGV[0].to_i)
      when 'list'
        list_vaults
      else
        puts "Unknown command: #{command}"
      end
    end

    def self.add_vault(path, mount_point)
      return puts "Usage: add <path> <mount_point>" unless path && mount_point

      print "Enter password: "
      password = gets.chomp

      store = VaultStore.new
      id = (store.all.map(&:id).max || 0) + 1
      vault = Vault.new(id: id, name: File.basename(path), path: path, mount_point: mount_point)

      if MountService.create_vault(path, password)
        KeyringService.store_password(id, password)
        store.add(vault)
        puts "Vault added successfully"
      else
        puts "Failed to create vault"
      end
    end

    def self.mount_vault(id)
      store = VaultStore.new
      vault = store.find(id)
      return puts "Vault not found" unless vault

      if MountService.mount(vault)
        puts "Vault mounted"
      else
        puts "Failed to mount"
      end
    end

    def self.unmount_vault(id)
      store = VaultStore.new
      vault = store.find(id)
      return puts "Vault not found" unless vault

      MountService.unmount(vault)
      puts "Vault unmounted"
    end

    def self.list_vaults
      store = VaultStore.new
      store.all.each do |vault|
        status = MountService.mounted?(vault.mount_point) ? 'mounted' : 'unmounted'
        puts "#{vault.id}: #{vault.name} (#{status})"
      end
    end
  end
end