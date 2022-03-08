window.BENCHMARK_DATA = {
  "lastUpdate": 1646719000225,
  "repoUrl": "https://github.com/alibaba/async_simple",
  "entries": {
    "C++ Benchmark": [
      {
        "commit": {
          "author": {
            "email": "68680648+ChuanqiXu9@users.noreply.github.com",
            "name": "Chuanqi Xu",
            "username": "ChuanqiXu9"
          },
          "committer": {
            "email": "68680648+ChuanqiXu9@users.noreply.github.com",
            "name": "Chuanqi Xu",
            "username": "ChuanqiXu9"
          },
          "distinct": true,
          "id": "54bd3c4c43b4163e00d64a9ac790689bee7ddb64",
          "message": "Enable Performance CI",
          "timestamp": "2022-03-08T13:52:45+08:00",
          "tree_id": "aff877f6ae5e21bf01111647305fbd7aaf52d95c",
          "url": "https://github.com/alibaba/async_simple/commit/54bd3c4c43b4163e00d64a9ac790689bee7ddb64"
        },
        "date": 1646718999611,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "async_simple_Lazy_chain",
            "value": 24355.384206787337,
            "unit": "ns/iter",
            "extra": "iterations: 29557\ncpu: 23774.682816253346 ns\nthreads: 1"
          },
          {
            "name": "async_simple_Lazy_collectAll",
            "value": 9914210.692307698,
            "unit": "ns/iter",
            "extra": "iterations: 65\ncpu: 9855347.692307692 ns\nthreads: 1"
          }
        ]
      }
    ]
  }
}