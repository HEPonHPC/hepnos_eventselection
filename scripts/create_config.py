# Copied from:
# https://github.com/hepnos/HEPnOS-Autotuning/blob/main/hepnos_theta/run.py#L93

import os
import json
import copy
import sys

def generate_hepnos_config_file(exp_dir='.',
                                filename='hepnos.json',
                                busy_spin=False,
                                use_progress_thread=False,
                                num_threads=0,
                                num_providers=1,
                                num_event_dbs=1,
                                num_product_dbs=1,
                                pool_type='fifo_wait'):
    hepnos_json_in = os.getcwd()+'/hepnos.json.in'
    hepnos_json = exp_dir + '/' + filename
    with open(hepnos_json_in) as f:
        config = json.loads(f.read())

    config['margo']['mercury']['na_no_block'] = bool(busy_spin)
    config['margo']['argobots']['pools'][0]['type'] = pool_type

    if use_progress_thread:
        config['margo']['argobots']['pools'].append({
            'name': '__progress__',
            'type': pool_type,
            'access': 'mpmc'
        })
        config['margo']['argobots']['xstreams'].append({
            'name': '__progress__',
            'scheduler': {
                'type': 'basic_wait',
                'pools': ['__progress__']
            }
        })
        config['margo']['progress_pool'] = '__progress__'
    else:
        config['margo']['progress_pool'] = '__primary__'

    rpc_pools = []
    for i in range(0, num_providers):
        config['margo']['argobots']['pools'].append({
            'name': ('__rpc_%d__' % i),
            'type': pool_type,
            'access': 'mpmc'
        })
        rpc_pools.append('__rpc_%d__' % i)

    if num_threads == 0:
        config['margo']['argobots']['xstreams'][0]['scheduler'][
            'pools'].extend(rpc_pools)
    else:
        es = []
        for i in range(0, min(num_threads, num_providers)):
            config['margo']['argobots']['xstreams'].append({
                'name': ('rpc_es_%d' % i),
                'scheduler': {
                    'type': 'basic_wait',
                    'pools': []
                }
            })
            es.append(config['margo']['argobots']['xstreams'][-1])
        for i in range(0, len(rpc_pools)):
            es[i % len(es)]['scheduler']['pools'].append(rpc_pools[i])

    ssg_group = None
    for g in config['ssg']:
        if g['name'] == 'hepnos':
            ssg_group = g
            break
    ssg_group['group_file'] = exp_dir + '/hepnos.ssg'

    event_db_model = {
        "type": "map",
        "comparator": "hepnos_compare_item_descriptors",
        "no_overwrite": True
    }
    product_db_model = {"type": "map", "no_overwrite": True}

    for i in range(0, num_providers):
        p = {
            "name": "hepnos_data_%d" % (i + 1),
            "type": "sdskv",
            "pool": rpc_pools[i % len(rpc_pools)],
            "provider_id": i + 1,
            "config": {
                "comparators": [{
                    "name": "hepnos_compare_item_descriptors",
                    "library": "libhepnos-service.so"
                }],
                "databases": []
            }
        }
        config['providers'].append(p)

    p = 0
    for i in range(0, num_event_dbs):
        event_db_name = 'hepnos-events-' + str(i)
        event_db = copy.deepcopy(event_db_model)
        event_db['name'] = event_db_name
        provider = config['providers'][1 + (p %
                                            (len(config['providers']) - 1))]
        provider['config']['databases'].append(event_db)
        p += 1

    for i in range(0, num_product_dbs):
        product_db_name = 'hepnos-products-' + str(i)
        product_db = copy.deepcopy(product_db_model)
        product_db['name'] = product_db_name
        provider = config['providers'][1 + (p %
                                            (len(config['providers']) - 1))]
        provider['config']['databases'].append(product_db)
        p += 1

    with open(hepnos_json, 'w+') as f:
        f.write(json.dumps(config, indent=4))

if __name__ == "__main__":

    if len(sys.argv)==5:
        n_threads    = int(sys.argv[1])
        n_providers  = int(sys.argv[2])
        n_event_dbs  = int(sys.argv[3])
        n_product_dbs= int(sys.argv[4])

        generate_hepnos_config_file(num_threads=n_threads, num_providers=n_providers, num_event_dbs=n_event_dbs, num_product_dbs=n_product_dbs)

    else:
        print("Usage: python create_config.py num_threads num_providers num_event_dbs num_product_dbs \n"
              "Note that the folder from where this script is run must contain a hepnos.json.in file.")
