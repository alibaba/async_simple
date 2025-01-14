export default {
  lang: 'en-US',
  title: 'yalantinglibs',
  description: 'Simple, light-weight and easy-to-use asynchronous components',
  base: '/async_simple/',
  lastUpdated: true,
  ignoreDeadLinks: false,
  outDir: "public",
  locales: {
    "/docs.en/": {
      lang: 'en-US',
      title: 'async_simple',
      description: 'Simple, light-weight and easy-to-use asynchronous components',
    },
    "/docs.cn/": {
      lang: 'zh-CN',
      title: 'async_simple',
      description: 'Simple, light-weight and easy-to-use asynchronous components',
    },
  },
  head: [],

  themeConfig: {
    nav: nav(),

    sidebar: {
      "/docs.en/": sidebarGuide(),
      '/docs.cn/': sidebarGuideZh(),
    },

    socialLinks: [
      {icon: 'github', link: 'https://github.com/alibaba/async_simple'}
    ],

    footer: {
      message: 'This website is released under the MIT License.',
      copyright: 'Copyright © 2022 yalantinglibs contributors'
    },

    editLink: {
      pattern: 'https://github.com/alibaba/async_simple/edit/main/docs/:path'
    }
  }
}

function nav() {
  return [
    {text: 'Guide', link: '/docs.en/GetStarted', activeMatch: '/guide/'},
    {
      text: "Language",
      items: [
        {
          text: "English", link: "/docs.en/GetStarted"
        },
        {
          text: "简体中文", link: '/docs.cn/GetStarted'
        }
      ]
    },
    {
      text: 'Github Issues',
      link: 'https://github.com/alibaba/async_simple/issues'
    }
  ]
}

function sidebarGuide() {
  return [ {
      text: 'async_simple',
      collapsible: true,
      items: [
        {
          text: 'Quick Start',
          collapsible: true,
          items: [
            {text: 'Get Started', link: '/docs.en/GetStarted'},
            {text: 'Stackless Coroutine', link: '/docs.en/StacklessCoroutine'},
            {text: 'Lazy', link: '/docs.en/Lazy'},
            {text: 'Debugging Lazy', link: '/docs.en/DebuggingLazy'},
            {text: 'Stackless Coroutine and Future', link: '/docs.en/StacklessCoroutineAndFuture'},
            {text: 'Try', link: '/docs.en/Try'},
            {text: 'Executor', link: '/docs.en/Executor'},
            {text: 'Signal And Cancellation', link: '/docs.en/SignalAndCancellation'},
            {text: 'Uthread', link: '/docs.en/Uthread'},
            {text: 'Interacting with Stackless Coroutine', link: '/docs.en/InteractingWithStacklessCoroutine'},
            {text: 'HybridCoro', link: '/docs.en/HybridCoro'},
            {text: 'Improve NetLib', link: '/docs.en/ImproveNetLibWithAsyncSimple'},
          ]
        },
        {
          text: 'Utils',
          collapsible: true,
          items: [
            {text: 'Future', link: '/docs.en/Future'},
            {text: 'Lock', link: '/docs.en/Lock'},
            {text: 'Latch', link: '/docs.en/Latch'},
            {text: 'ConditionVariable', link: '/docs.en/ConditionVariable'},
            {text: 'Semaphore', link: '/docs.en/Semaphore'},
          ]
        },
        {
          text: 'Performance',
          collapsible: true,
          items: [
            {text: 'Performance', link: '/docs.en/Performance'},
            {text: 'Quantitative Analysis of Performance', link: '/docs.en/QuantitativeAnalysisReportOfCoroutinePerformance'},
          ]
        },
      ]
    },
    {
      text: 'struct_pack',
      link: 'https://alibaba.github.io/yalantinglibs/guide/struct-pack-intro.html'
    },
    {
      text: 'struct_pb',
      link: 'https://alibaba.github.io/yalantinglibs/guide/struct-pb-intro.html'
    },
    {
      text: 'coro_rpc',
      link: 'https://alibaba.github.io/yalantinglibs/guide/coro-rpc-intro.html'
    }
  ]
}

function sidebarGuideZh() {
  return [
    {
      text: 'async_simple',
      collapsible: true,
      items: [
        {
          text: '快速入门',
          collapsible: true,
          items: [
            {text: 'Get Started', link: '/docs.cn/GetStarted'},
            {text: '无栈协程', link: '/docs.cn/无栈协程'},
            {text: 'Lazy', link: '/docs.cn/Lazy'},
            {text: '调试Lazy', link: '/docs.cn/调试 Lazy'},
            {text: '协程与异步风格比较', link: '/docs.cn/协程与异步风格比较'},
            {text: 'Try', link: '/docs.cn/Try'},
            {text: 'Executor', link: '/docs.cn/Executor'},
            {text: '信号与任务的取消', link: '/docs.cn/信号与任务的取消'},
            {text: 'Uthread', link: '/docs.cn/Uthread'},
            {text: '与无栈协程交互', link: '/docs.cn/与无栈协程交互'},
            {text: 'HybridCoro', link: '/docs.cn/HybridCoro'},
            {text: '用async_simple改造网络库', link: '/docs.cn/用 async_simple 改造网络库'},
          ]
        },
        {
          text: '工具类',
          collapsible: true,
          items: [
            {text: 'Future', link: '/docs.cn/Future'},
            {text: 'Lock', link: '/docs.cn/Lock'},
            {text: 'Latch', link: '/docs.cn/Latch'},
            {text: 'ConditionVariable', link: '/docs.cn/ConditionVariable'},
            {text: 'Semaphore', link: '/docs.cn/Semaphore'},
          ]
        },
        {
          text: '性能',
          collapsible: true,
          items: [
            {text: 'Performance', link: '/docs.cn/Performance'},
            {text: '协程性能定量分析', link: '/docs.cn/基于async_simple的协程性能定量分析'},
          ]
        },
      ]
    },
    {
      text: 'struct_pack',
      link: 'https://alibaba.github.io/yalantinglibs/guide/struct-pack-intro.html'
    },
    {
      text: 'struct_pb',
      link: 'https://alibaba.github.io/yalantinglibs/guide/struct-pb-intro.html'
    },
    {
      text: 'coro_rpc',
      link: 'https://alibaba.github.io/yalantinglibs/guide/coro-rpc-intro.html'
    }
  ]
}
