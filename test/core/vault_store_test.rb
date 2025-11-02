require_relative '../test_helper'
require 'fileutils'
require_relative '../../lib/guicrypt_tui/core/vault_store'

describe GuicryptTui::VaultStore do
  let(:store) { GuicryptTui::VaultStore.new }
  let(:vault) { GuicryptTui::Vault.new(id: 1, name: 'test', path: '.', mount_point: '/tmp') }

  before do
    File.delete('vaults.json') if File.exist?('vaults.json')
  end

  it 'loads empty if no file' do
    expect(store.all).to be_empty
  end

  it 'adds vault' do
    store.add(vault)
    expect(store.all.size).to eq(1)
    expect(store.find(1).name).to eq('test')
  end

  it 'saves to json' do
    store.add(vault)
    data = JSON.parse(File.read('vaults.json'))
    expect(data.first['name']).to eq('test')
  end

  it 'removes vault' do
    store.add(vault)
    store.remove(1)
    expect(store.all).to be_empty
  end
end