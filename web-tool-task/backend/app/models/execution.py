# Execution model
from sqlalchemy import Column, Integer, String, DateTime
from ..core.database import Base

class Execution(Base):
    __tablename__ = "executions"

    id = Column(Integer, primary_key=True, index=True)
    task_id = Column(Integer)
    status = Column(String)
    executed_at = Column(DateTime)
