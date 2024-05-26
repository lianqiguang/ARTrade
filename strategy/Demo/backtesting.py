import enum
import pandas as pd
import matplotlib.pyplot as plt


class BackTest(object):
    class Direction(enum.Enum):
        # 做空
        Short = -1
        # 做多
        Long = 1

    class Action(enum.Enum):
        # 开仓
        Open = -1
        # 平仓
        Close = 1

    class Order(object):
        def __init__(self, lever) -> None:
            # 杠杆倍数
            self.lever: int = lever
            self.reset()

        def reset(self):
            # 标志位
            self.used = False
            # 开仓时间
            self.open_time: str|int = 0
            # 开仓价
            self.open_price: float = 0.0
            # 开仓数量
            self.open_volume: float = 0.0
            # 开仓方向：做多 | 做空
            self.open_direction: BackTest.Direction = None

    def __init__(self, asset: float, ratio: float, stop_ratio: float, fee: float, lever: int) -> None:
        # 账户资产
        self.asset = asset
        # 每次开仓金额占账户资产比例
        self.ratio = ratio
        # 止损比例
        self.stop_ratio = stop_ratio
        # 手续费
        self.fee = fee
        # 杠杆倍数
        self.lever = lever

        # 数量精度
        self.precision = 10 ** 3

        # 动态盈亏
        self.profit_and_lost: float = 0.0
        # 收益率
        self.earning_ratio: float = 0.0
        # 锁定资产(持仓资产)
        self.lock_asset: float = 0.0
        # 订单详情
        self.order = BackTest.Order(self.lever)

    # 收益率
    def EarningRate(self, last_price: float) -> float:
        if not self.order.used:
            return 0.0
        return ((last_price - self.order.open_price) / self.order.open_price) * self.order.open_direction.value
 
    # 收益值
    def Earning(self, last_price: float) -> float:
        if not self.order.used:
            return 0.0
        return (last_price - self.order.open_price) * self.order.open_direction.value * self.order.open_volume * (1 - self.fee)
 
    # 平仓: 计算收益率 & 更新净值
    def OpenPosition(self, last_price: float, direction: Direction, time: int|str):
        self.order.used = True

        # 含手续费价格
        price_with_fee = last_price * (1 + self.fee)
        self.order.open_volume = ((self.asset * self.ratio * self.lever) / price_with_fee) * self.precision / self.precision
        self.order.open_price = last_price
        self.order.open_direction = direction
        self.order.open_time = time

        # 资产处置
        self.asset = self.asset - (self.order.open_volume * self.order.open_price)
        self.lock_asset = self.order.open_volume * self.order.open_price
        pass

    # 开仓：计算收益率 & 更新净值
    def ClosePosition(self, last_price: float):
        """ 当前回测版本策略为全部平仓 """
        if not self.order.used:
            return
     
        # 计算收益率
        self.earning_ratio = self.EarningRate(last_price=last_price)

        # 更新净值
        self.asset = self.asset + self.lock_asset + self.Earning(last_price=last_price)
        self.lock_asset = 0.0
        self.order.reset()
        pass

    # 播放Kline行情
    def on_kiline(self, time: str|int, open: float, close: float, high: float, low: float, vloume: float, signal: int):
        if signal == 0:
            # 更新动态盈亏 和 持仓资产
            self.profit_and_lost = self.Earning(last_price=close)
            self.lock_asset = self.order.open_price * self.order.open_volume

            # 止损
            if self.EarningRate(last_price=close) <= self.stop_ratio:
                self.ClosePosition(last_price=close)
            pass
        elif signal == 1:
            # 做多
            self.ClosePosition(last_price=close)
            self.OpenPosition(last_price=close, direction=BackTest.Direction.Long, time=time)
            pass
        elif signal == -1:
            # 做空
            self.ClosePosition(last_price=close)
            self.OpenPosition(last_price=close, direction=BackTest.Direction.Short, time=time)
            pass

    def run(self, file: str):
        df = pd.read_csv(file)
        df['open_time'] = pd.to_datetime(df['open_time'])
        result = list()
        for item in df.itertuples():
            self.on_kiline(item.open_time, item.Open, item.Close, item.High, item.Low, item.Volume, item.trade_point)
            response = {"open_time": item.open_time, "Signal": item.trade_point, "Asset": self.asset, "LockAsset": self.lock_asset, "All": self.asset + self.lock_asset}
            result.append(response)

        df_result = pd.DataFrame(result)
        print(df_result)
        self.show(df=df_result)

    def show(self, df: pd.DataFrame):
        plt.figure(figsize=(10, 6))
        plt.plot(df['open_time'].to_numpy(), df['All'].to_numpy())
        plt.title("BackTest")
        plt.xlabel('Time')
        plt.ylabel('Capital')
        plt.xticks(rotation=15)
        plt.grid(True)
        plt.savefig(f'AccountFund_x{lever}.png')


if __name__ == "__main__":
    init_asset = 10000
    ratio = 1
    fee = 0.0005
    lever = 1
    stop_ratio = 0.01

    back_test = BackTest(
        asset=init_asset,
        ratio=ratio,
        stop_ratio=stop_ratio,
        fee=fee,
        lever=lever
    )

    back_test.run("/home/ubuntu/ARTrade/strategy/Demo/talib.csv")