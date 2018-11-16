version_file_name = 'VERSION'

try:
    # read old version
    with open(version_file_name, 'r') as version_file:
        old_version=version_file.read().replace('\n','')
    # print old version
    print('Old version: ' + old_version)
except IOError:
    # create version file is it doesn't exist
    with open(version_file_name, 'w') as version_file:
        print('Version file has been created.')
        
done = False
while not done:
    # request new version
    new_version = input('New version: ')

    # check new version
    version_indices = new_version.split('.')
    if (len(version_indices) == 3 and
        version_indices[0].isdigit() and
        version_indices[1].isdigit() and
        version_indices[2].isdigit()):
        # record new version
        with open(version_file_name, 'w') as version_file:
            version_file.write(new_version)
        done = True
    else:
        print('Invalid new version!')
