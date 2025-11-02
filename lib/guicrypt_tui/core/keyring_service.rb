require 'json'

module GuicryptTui
  class KeyringService
    PASSWORD_FILE = 'passwords.json'

    def self.store_password(vault_id, password)
      data = load_passwords
      data[vault_id.to_s] = password
      save_passwords(data)
    end

    def self.get_password(vault_id)
      load_passwords[vault_id.to_s]
    end

    private

    def self.load_passwords
      File.exist?(PASSWORD_FILE) ? JSON.parse(File.read(PASSWORD_FILE)) : {}
    end

    def self.save_passwords(data)
      File.write(PASSWORD_FILE, JSON.generate(data))
    end
  end
end